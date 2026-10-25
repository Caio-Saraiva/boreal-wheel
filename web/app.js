'use strict';
const $ = id => document.getElementById(id);
const labels = {0:'D-pad ↑',1:'D-pad ↓',2:'D-pad ←',3:'D-pad →',4:'Start',5:'Back',6:'LS',7:'RS',8:'LB',9:'RB',12:'A',13:'B',14:'X',15:'Y'};
const bits = Object.keys(labels).map(Number);
const pageInfo = {overview:['De volta à pista','Seu Ferrari 458 Italia. A mesma sensação, uma nova conexão.'],mapping:['Todos os comandos, do seu jeito','Escolha como o seu volante se apresenta ao jogo.'],calibration:['Precisão em cada curva','Encontre a resposta que combina com a sua pilotagem.'],diagnostics:['Uma conexão transparente','Acompanhe a leitura, os eixos e a saída para o jogo.']};
let state, page='overview', configSignature='', history=[], toastTimer, online=false;
let configQueue=Promise.resolve(), dirty=new Set(), manualButtons=0;
const manual={steering:0,throttle:0,brake:0,ly:0,rx:0,ry:0,buttons:0};
const keys=new Set();
function icon(name){const svg=document.createElementNS('http://www.w3.org/2000/svg','svg');const use=document.createElementNS(svg.namespaceURI,'use');use.setAttribute('href','#i-'+name);svg.append(use);return svg;}
function toast(message){$('toast').textContent=message;$('toast').classList.add('visible');clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('toast').classList.remove('visible'),3500);}
async function post(path,params){const response=await fetch('/api/'+path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(params).toString(),signal:AbortSignal.timeout(2500)});if(!response.ok)throw new Error(await response.text());return response.json();}
async function action(name){try{await flushStudioSave();await configQueue;await post('action',{action:name});if(name==='reset'){studio.draft={};configSignature='';toast('Perfil padrão restaurado.');}if(name.startsWith('center'))toast('Posição central salva.');if(name.startsWith('limit'))toast('Fim do curso capturado.');await refresh();}catch(e){toast(e.message);}}
function setPage(name){page=name;document.querySelectorAll('.page-panel').forEach(el=>el.hidden=el.id!=='page-'+name);document.querySelectorAll('[data-page]').forEach(el=>{const active=el.dataset.page===name;el.classList.toggle('active',active);if(active)el.setAttribute('aria-current','page');else el.removeAttribute('aria-current');});$('page-title').replaceChildren(document.createTextNode(pageInfo[name][0]));const dot=document.createElement('span');dot.textContent='.';$('page-title').append(dot);$('page-subtitle').textContent=pageInfo[name][1];if(state)render(state);requestAnimationFrame(drawCharts);}
document.querySelectorAll('[data-page]').forEach(el=>el.addEventListener('click',()=>setPage(el.dataset.page)));
$('output-details').onclick=()=>setPage('diagnostics');
$('physical-mode').onclick=()=>action('physical');$('demo-mode').onclick=()=>action('demo');
$('bridge-toggle').onclick=()=>{if(!state)return;if(state.platform!=='windows'){setPage('diagnostics');toast('A saída XInput requer a versão Windows e o ViGEmBus.');return;}action(state.output_requested?'stop':'start');};
$('demo-interaction').onclick=async()=>{await action(state?.demo_auto?'manual':'auto');};

let resetPending=false;
$('reset-profile').onclick=()=>{if(!resetPending){resetPending=true;$('reset-profile').textContent='Confirmar restauração';setTimeout(()=>{resetPending=false;$('reset-profile').textContent='Restaurar padrão';},5000);return;}resetPending=false;$('reset-profile').textContent='Restaurar padrão';action('reset');};
function openHelp(){$('help-dialog').showModal();}
$('help-open').onclick=openHelp;$('help-top').onclick=openHelp;
$('help-close').onclick=()=>$('help-dialog').close();$('help-done').onclick=()=>$('help-dialog').close();
$('help-dialog').addEventListener('click',e=>{if(e.target===$('help-dialog')){const r=e.target.getBoundingClientRect();if(e.clientX<r.left||e.clientX>r.right||e.clientY<r.top||e.clientY>r.bottom)e.target.close();}});
for(const bit of [12,13,14,15,8,9,5,4]){const pill=document.createElement('span');pill.className='button-pill'+(bit>=12?' face':'');pill.dataset.bit=bit;pill.textContent=labels[bit].toUpperCase();pill.title=labels[bit];$('button-pills').append(pill);}
buildStudio();
for(let i=0;i<6;i++){const el=document.createElement('div');el.className='raw-axis';const label=document.createElement('span');label.textContent=['X / LX','Y / LY','Z / LT','RX','RY','RZ / RT'][i];const value=document.createElement('code');value.id='raw-axis-'+i;value.textContent='0.000';el.append(label,value);$('raw-axes').append(el);}
const diagnosticFields=[['mode','Modo de operação'],['device','Dispositivo informado'],['backend','API de entrada'],['packets','Iterações do loop'],['poll','Loop observado / alvo'],['map','Remapeamento C'],['output','Saída virtual']];
for(const [key,label] of diagnosticFields){const row=document.createElement('div');row.className='diagnostic-item';const name=document.createElement('span');name.textContent=label;const value=document.createElement('strong');value.id='diag-'+key;row.append(name,value);$('diagnostic-list').append(row);}
function queueSetting(key,value){return queueSettings({[key]:value});}
function syncConfig(c){syncStudio(c);}
function reportValue(el,v,unit){$(el).replaceChildren(document.createTextNode(v));const span=document.createElement('span');span.textContent=unit;$(el).append(span);}
function metric(el,v,unit){$(el).replaceChildren(document.createTextNode(v));const small=document.createElement('small');small.textContent=unit;$(el).append(small);}
function render(s){syncConfig(s.config);const available=Boolean(s.connected),physical=Boolean(s.physical_connected),demo=Boolean(s.demo);$('physical-mode').classList.toggle('selected',!demo);$('demo-mode').classList.toggle('selected',demo);$('device-badge').className='badge'+(demo?' demo':physical?' live':'');$('device-badge').replaceChildren();const dot=document.createElement('span');dot.className='status-dot';$('device-badge').append(dot,document.createTextNode(demo?'Demonstração':physical?'Entrada detectada':s.usb_present?'USB detectado':'Aguardando USB'));$('source-label').textContent=s.backend||(s.usb_present?'USB presente · sem leitura':'Procurando dispositivo…');$('device-detail').textContent=demo?'Volante + pedais · simulação para prévia':physical?s.name:s.usb_present?'Volante USB presente · verifique o driver':'Volante + pedais · aguardando conexão USB';$('demo-notice').hidden=!demo;$('physical-notice').hidden=demo||physical;$('physical-notice-text').textContent=physical?'':s.detail||'Conecte o volante ou ative Demo para explorar.';$('manual-controls').hidden=!demo||Boolean(s.demo_auto);$('demo-interaction').textContent=s.demo_auto?'Usar teclado e mouse ↗':'Reproduzir demonstração ↗';

const button=$('bridge-toggle');button.disabled=demo||!online||(s.platform==='windows'&&!physical&&!s.output_requested);button.querySelector('span').textContent=s.output_requested?'Desativar ponte':s.platform==='linux'?'XInput no Windows':'Ativar ponte';button.setAttribute('aria-pressed',String(Boolean(s.output_requested)));
const out=s.report,steer=available?out.lx/(out.lx<0?32768:32767):0;const r={...s.raw,steering:steer,throttle:out.rt/255,brake:out.lt/255};
$('wheel-rotation').setAttribute('transform',`rotate(${steer*135} 310 175)`);
const angle=steer*135,rad=angle*Math.PI/180;const px=310+153*Math.sin(rad),py=175-153*Math.cos(rad);$('direction-arc').setAttribute('d',`M310 22A153 153 0 0 ${angle>=0?1:0} ${px.toFixed(2)} ${py.toFixed(2)}`);$('direction-arc').setAttribute('opacity',Math.abs(angle)<.1?'0':'1');
reportValue('steering-deg',(steer*135).toFixed(1),'°');$('lx-small').textContent=out.lx;$('axis-marker').style.left=(steer+1)*50+'%';$('steering-normalized').textContent=steer.toFixed(3);
for(const name of ['throttle','brake']){const value=available?r[name]:0;$(name+'-fill').style.height=value*100+'%';$(name+'-plate').style.transform=`perspective(130px) rotateX(${11+value*22}deg) translateY(${value*4}px)`;reportValue(name+'-value',Math.round(value*100),'%');}
$('rt-value').textContent=String(out.rt).padStart(3,'0');$('lt-value').textContent=String(out.lt).padStart(3,'0');$('pedal-caption').textContent=!available?'Aguardando leitura dos eixos':s.combined_pedals?'Pedais combinados · limite do DirectInput':demo?'Eixos simulados independentes':'Saída calibrada · acelerador RT / freio LT';
document.querySelectorAll('[data-bit]').forEach(el=>el.classList.toggle('active',Boolean(r.buttons&(1<<Number(el.dataset.bit)))));document.querySelectorAll('[data-map-bit]').forEach(el=>el.classList.toggle('active',Boolean(r.buttons&(1<<Number(el.dataset.mapBit)))));$('input-dot').classList.toggle('active',available&&Boolean(r.buttons||Math.abs(steer)>.02||r.throttle>.01||r.brake>.01));
metric('processing-value',s.map_us.toFixed(1),'µs');metric('poll-value',Math.round(s.observed_hz),'Hz');$('poll-target').textContent='Alvo: '+s.config.poll_hz+' Hz';$('output-card-status').textContent=s.output_status;$('footer-platform').textContent=s.platform==='linux'?'Linux preview':'Windows bridge';
$('diag-mode').textContent=demo?'Simulação · sem saída':'Hardware físico';$('diag-device').textContent=s.name||'Nenhum dispositivo legível';$('diag-backend').textContent=s.backend||'Aguardando';$('diag-packets').textContent=s.packets.toLocaleString('pt-BR');$('diag-poll').textContent=Math.round(s.observed_hz)+' / '+s.config.poll_hz+' Hz';$('diag-map').textContent=s.map_us.toFixed(3)+' µs (só transformação)';$('diag-output').textContent=s.output_active?'Ativa':'Inativa';$('diagnostic-detail').textContent=s.detail||'Aguardando entrada do dispositivo.';$('diagnostic-output').textContent=s.output_status;renderStudio(s);
r.axes.forEach((v,i)=>$('raw-axis-'+i).textContent=v.toFixed(3));for(const [side,x,y] of [['left',out.lx,out.ly],['right',out.rx,out.ry]]){$('stick-'+side).style.left=(50+x/32768*44)+'%';$('stick-'+side).style.top=(50-y/32768*44)+'%';$('stick-'+side+'-value').textContent=x+' / '+y;}
}
function setupCanvas(id){const canvas=$(id),box=canvas.getBoundingClientRect();if(!box.width||!box.height)return null;const dpr=window.devicePixelRatio||1;const w=Math.round(box.width*dpr),h=Math.round(box.height*dpr);if(canvas.width!==w||canvas.height!==h){canvas.width=w;canvas.height=h;}const ctx=canvas.getContext('2d');ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,box.width,box.height);return {ctx,w:box.width,h:box.height};}
function drawSignal(){const data=setupCanvas('signal-canvas');if(!data)return;const {ctx,w,h}=data;ctx.strokeStyle='#c4dccc55';ctx.lineWidth=.6;for(let i=1;i<4;i++){ctx.beginPath();ctx.moveTo(0,h*i/4);ctx.lineTo(w,h*i/4);ctx.stroke();}if(!history.length)return;ctx.beginPath();history.forEach((v,i)=>{const x=w*i/(history.length-1||1),y=h/2-v*(h*.39);i?ctx.lineTo(x,y):ctx.moveTo(x,y);});ctx.strokeStyle='#6ba88a';ctx.lineWidth=1.3;ctx.stroke();}
function drawCharts(){if(page==='overview')drawSignal();if(page==='calibration')drawCalibration();}
window.addEventListener('resize',drawCharts);
let refreshing=false;
async function refresh(){if(refreshing)return;refreshing=true;try{const response=await fetch('/api/state',{signal:AbortSignal.timeout(1800),cache:'no-store'});if(!response.ok)throw new Error('Core offline');state=await response.json();online=true;$('connection-error').hidden=true;render(state);history.push(state.report.lx/32768);if(history.length>180)history.shift();drawCharts();}catch(e){online=false;$('connection-error').hidden=false;$('bridge-toggle').disabled=true;$('device-badge').textContent='Núcleo offline';$('device-badge').className='badge';$('source-label').textContent='Sem conexão com o processo C';$('output-card-status').textContent='Sem telemetria · verifique o processo C';}finally{refreshing=false;}}
async function loop(){await refresh();setTimeout(loop,document.hidden?300:40);}loop();
$('export-diagnostics').onclick=()=>{if(!state)return;const blob=new Blob([JSON.stringify({...state,exported_at:new Date().toISOString()},null,2)],{type:'application/json'});const url=URL.createObjectURL(blob),a=document.createElement('a');a.href=url;a.download='boreal-wheel-diagnostico.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);toast('Diagnóstico exportado.');};
let demoTimer, demoBusy=false,demoPending=false;
function sendManual(){demoPending=true;if(demoTimer)return;demoTimer=setTimeout(flushManual,30);}
async function flushManual(){demoTimer=null;if(demoBusy||!state?.demo||state.demo_auto)return;demoBusy=true;demoPending=false;try{manual.buttons=manualButtons;await post('demo',manual);}catch(e){toast(e.message);}finally{demoBusy=false;if(demoPending)sendManual();}}
for(const name of ['steering','throttle','brake','ly','rx','ry'])$('demo-'+name).oninput=e=>{manual[name]=Number(e.target.value);sendManual();};
function keyboardInput(){manual.steering=(keys.has('d')?1:0)-(keys.has('a')?1:0);manual.throttle=keys.has('w')?1:0;manual.brake=keys.has('s')?1:0;manualButtons=0;for(const [key,bit] of [['j',12],['k',13],['u',14],['i',15],['q',8],['e',9],['arrowup',0],['arrowdown',1],['arrowleft',2],['arrowright',3]])if(keys.has(key))manualButtons|=1<<bit;for(const name of ['steering','throttle','brake'])$('demo-'+name).value=manual[name];sendManual();}
window.addEventListener('keydown',e=>{if(!state?.demo||state.demo_auto||$('help-dialog').open||['INPUT','SELECT','TEXTAREA'].includes(document.activeElement?.tagName))return;const key=e.key.toLowerCase();if(['a','d','w','s','j','k','u','i','q','e','arrowup','arrowdown','arrowleft','arrowright'].includes(key)){e.preventDefault();keys.add(key);keyboardInput();}});
window.addEventListener('keyup',e=>{if(keys.delete(e.key.toLowerCase()))keyboardInput();});
window.addEventListener('blur',()=>{keys.clear();if(state?.demo&&!state.demo_auto){Object.assign(manual,{steering:0,throttle:0,brake:0,ly:0,rx:0,ry:0,buttons:0});manualButtons=0;keyboardInput();}});

// Mapping and calibration studio. All game reports are still generated by C.
var studio;
function buildStudio(){
  studio={draft:{},versions:{},sequence:0,timer:null,learn:null,measure:null};
  studio.axisNames=['Direção','Analógico esquerdo · vertical','Analógico direito · horizontal','Analógico direito · vertical','Acelerador','Freio'];
  studio.outputNames=['LX','LY','RX','RY','RT','LT'];
  studio.sourceNames=['Volante / esquerdo horizontal','Esquerdo vertical','Direito horizontal','Direito vertical','Pedal 1 · RT recebido','Pedal 2 · LT recebido'];
  studio.sources=[...Array.from({length:6},(_,i)=>i),...bits.map(i=>i+6),...Array.from({length:6},(_,i)=>i+22)];
  const element=(tag,cls,text)=>{const el=document.createElement(tag);if(cls)el.className=cls;if(text)el.textContent=text;return el;};
  for(const i of [0,4,5,1,2,3]){
    const card=element('article','glass route-card');card.dataset.routeAxis=i;
    const heading=element('div','route-heading');heading.append(element('h3','',studio.axisNames[i]),element('code','',studio.outputNames[i]+' → jogo'));card.append(heading);
    card.append(sourceField('axis_source_'+i,'Receber movimento de',true));
    if(i<4){const detail=element('details','negative-route');detail.append(element('summary','','Usar duas entradas para os dois lados'));detail.append(sourceField('axis_negative_'+i,'Entrada para o lado negativo',true));card.append(detail);}
    const meter=element('div','route-meter');meter.innerHTML='<i></i><span></span>';card.append(meter);
    $('axis-routes').append(card);
    const panel=element('article','glass calibration-card');panel.dataset.calAxis=i;
    panel.innerHTML=`<div class="route-heading"><div><span class="section-index">${studio.outputNames[i]}</span><h2>${studio.axisNames[i]}</h2></div><span class="live-label"><span class="status-dot"></span>AO VIVO</span></div>
      <p class="cal-source" id="cal-source-${i}"></p>
      <canvas id="cal-canvas-${i}" class="calibration-canvas" role="img" aria-label="Calibração visual de ${studio.axisNames[i]}"></canvas>
      <p class="drag-hint">Arraste os pontos <b>laranja</b> para ignorar ruído e os <b>azuis</b> para ajustar o fim do curso.</p>
      <div class="cal-live"><span>Entrada <b id="cal-in-${i}">0%</b></span><span>Saída <b id="cal-out-${i}">0%</b></span><strong id="cal-gate-${i}">Em repouso</strong></div>
      <div class="cal-controls"></div>
      <div class="cal-actions"><button class="secondary-button" data-measure="${i}">Medir repouso</button><button class="secondary-button" data-limit="${i}">Capturar máximo</button>${i<4?`<button class="secondary-button" data-center="${i}"${i===0?' id="calibrate-center"':''}>Capturar centro</button>`:''}</div>
      <label class="switch-row">Inverter sentido<input type="checkbox" data-setting="axis_invert_${i}"><span class="toggle"></span></label>`;
    const controls=panel.querySelector('.cal-controls');
    controls.append(adjustment('Zona morta','axis_deadzone_'+i,0,60,.1,100,'%','Movimentos dentro desta área são ignorados.',i===0?'deadzone':null));
    controls.append(adjustment('100% de saída ao mover','axis_limit_'+i,5,100,.1,100,'%','Diminua para deixar mais sensível. O restante do curso continua em 100%.'));
    controls.append(adjustment('Curva de resposta','axis_gamma_'+i,.2,3,.05,1,'','Abaixo de 1: responde antes. 1: linear. Acima de 1: mais delicado no início.',i===0?'gamma':null));
    if(i<4){const detail=element('details','center-detail');detail.append(element('summary','','Ajustar o centro por número'));detail.append(adjustment('Centro','axis_center_'+i,-50,50,.1,100,'%','Prefira capturar com o controle em repouso.'));controls.append(detail);}
    (i===0||i>=4?$('primary-calibration'):$('extra-calibration')).append(panel);
    bindCanvas(panel.querySelector('canvas'),i);
  }
  for(const bit of bits){
    const card=element('div','mapping-row button-route');card.dataset.outputBit=bit;
    const title=element('div','');title.append(element('i','map-light'),element('strong','',labels[bit]+' → jogo'));card.append(title);
    card.append(sourceField('button_source_'+bit,'Receber de',true));
    const detail=element('details','threshold-detail');detail.append(element('summary','','Limite para eixos / pedais'));
    const select=element('select');select.dataset.setting='button_direction_'+bit;select.setAttribute('aria-label','Lado do eixo para '+labels[bit]);
    select.innerHTML='<option value="1">Lado positivo / pressionado</option><option value="-1">Lado negativo</option>';detail.append(select);
    detail.append(adjustment('Acionar ao passar de','button_threshold_'+bit,1,100,1,100,'%','Botões físicos acionam imediatamente.'));card.append(detail);
    $('mapping-rows').append(card);
  }
  document.querySelectorAll('.swap-pedals').forEach(button=>button.onclick=()=>{
    if(!state)return;const c=studioConfig();queueSettings({axis_source_4:c.axis_source_5,axis_source_5:c.axis_source_4});toast('Entradas do acelerador e do freio trocadas.');
  });
  document.querySelectorAll('[data-setting]').forEach(bindSetting);
  document.querySelectorAll('[data-measure]').forEach(button=>button.onclick=()=>startMeasure(Number(button.dataset.measure),button));
  document.querySelectorAll('[data-center]').forEach(button=>button.onclick=()=>action('center_'+button.dataset.center));
  document.querySelectorAll('[data-limit]').forEach(button=>button.onclick=()=>action('limit_'+button.dataset.limit));
  document.querySelectorAll('details').forEach(el=>el.addEventListener('toggle',()=>requestAnimationFrame(drawCharts)));
  setupProfiles();setupTheme();
}
function sourceName(source){
  if(source<0)return 'Desativado';
  if(source<6)return studio.sourceNames[source];
  if(source<22)return 'Botão '+labels[source-6];
  return 'Eixo bruto '+['X','Y','Z','RX','RY','RZ'][source-22];
}
function sourceField(key,label,learn){
  const group=document.createElement('div');group.className='source-field';
  const field=document.createElement('label');field.textContent=label;field.className='field-label';field.htmlFor='source-'+key;
  const row=document.createElement('div');row.className='source-select';
  const select=document.createElement('select');select.id='source-'+key;select.dataset.setting=key;
  const off=document.createElement('option');off.value=-1;off.textContent='Desativado';select.append(off);
  for(const [label,list] of [['Volante, pedais e analógicos',studio.sources.filter(i=>i<6)],['Botões e direcional',studio.sources.filter(i=>i>=6&&i<22)],['Avançado · eixos diretos do dispositivo',studio.sources.filter(i=>i>=22)]]){const group=document.createElement('optgroup');group.label=label;for(const source of list){const option=document.createElement('option');option.value=source;option.textContent=sourceName(source);group.append(option);}select.append(group);}
  row.append(select);
  if(learn){const button=document.createElement('button');button.className='learn-button';button.textContent='Identificar';button.type='button';button.setAttribute('aria-label','Identificar entrada de '+key);button.onclick=()=>startLearn(key,button);row.append(button);}
  group.append(field,row);return group;
}
function adjustment(label,key,min,max,step,scale,unit,help,id){
  const block=document.createElement('div');block.className='adjustment';
  const sliderId=id||'range-'+key;
  const heading=document.createElement('div');heading.className='adjustment-label';
  const name=document.createElement('label');name.htmlFor=sliderId;name.textContent=label;
  const field=document.createElement('label');field.className='number-field';
  const number=document.createElement('input');number.type='number';number.min=min;number.max=max;number.step='any';number.dataset.setting=key;number.dataset.scale=scale;number.setAttribute('aria-label',label+' · valor numérico');
  field.append(number,document.createTextNode(unit));heading.append(name,field);
  const range=document.createElement('input');range.type='range';range.id=sliderId;range.min=min;range.max=max;range.step=step;range.dataset.setting=key;range.dataset.scale=scale;range.setAttribute('aria-label',label);
  const text=document.createElement('p');text.className='control-help';text.textContent=help;
  block.append(heading,range,text);return block;
}
function studioConfig(){return {...state?.config,...studio.draft};}
function bindSetting(el){
  const stage=()=>{
    if(el.type==='number'&&!el.checkValidity())return;
    const key=el.dataset.setting,scale=Number(el.dataset.scale)||1;
    let value=el.type==='checkbox'?Number(el.checked):Number(el.value)/scale;
    if(!Number.isFinite(value))return;
    const c=studioConfig(),i=Number(key.split('_').at(-1));
    if(key.startsWith('axis_deadzone_'))value=Math.min(value,c['axis_limit_'+i]-.02);
    if(key.startsWith('axis_limit_'))value=Math.max(value,c['axis_deadzone_'+i]+.02);
    studio.draft[key]=Number(value.toFixed(6));
    if(key.startsWith('button_source_')&&value>=4&&value<22)studio.draft[key.replace('source','direction')]=1;
    syncStudio(state?.config||{});drawCharts();
    return {[key]:studio.draft[key]};
  };
  el.addEventListener('input',()=>{if(el.type==='range'){stage();scheduleStudioSave();}});
  el.addEventListener('change',()=>{const change=stage();if(change){flushStudioSave();}else{toast('Digite um número dentro do intervalo indicado.');el.value=String((studioConfig()[el.dataset.setting]||0)*(Number(el.dataset.scale)||1));}});
}
function scheduleStudioSave(){if(studio.timer)return;studio.timer=setTimeout(flushStudioSave,120);}
function flushStudioSave(){clearTimeout(studio.timer);studio.timer=null;const patch={...studio.draft};if(Object.keys(patch).length)return queueSettings(patch);return configQueue;}
function queueSettings(patch){
  if(!state)return Promise.resolve();
  const tag=++studio.sequence;
  for(const [key,value] of Object.entries(patch)){studio.draft[key]=value;studio.versions[key]=tag;dirty.add(key);}
  $('save-status').textContent='Salvando…';
  configQueue=configQueue.catch(()=>{}).then(async()=>{
    try{await post('config',patch);$('save-status').textContent='Perfil salvo neste computador';}
    catch(error){$('save-status').textContent='Falha ao salvar';toast('Não foi possível aplicar o ajuste: '+error.message);}
    finally{for(const key of Object.keys(patch)){if(studio.versions[key]===tag&&studio.draft[key]===patch[key]){delete studio.draft[key];dirty.delete(key);}}configSignature='';await refresh();}
  });return configQueue;
}
function syncStudio(config){
  const c={...config,...studio.draft};
  document.querySelectorAll('[data-setting]').forEach(el=>{
    if(el===document.activeElement && el.type==='number')return;
    const value=c[el.dataset.setting];if(value===undefined)return;
    if(el.type==='checkbox')el.checked=Boolean(value);
    else el.value=String(Number((value*(Number(el.dataset.scale)||1)).toFixed(3)));
  });
  for(let i=0;i<6;i++){
    const source=c['axis_source_'+i];$('cal-source-'+i).textContent=sourceName(source)+' → '+studio.outputNames[i];
    document.querySelectorAll(`[data-setting="axis_deadzone_${i}"][type=range]`).forEach(el=>el.max=Math.max(0,Math.min(60,(c['axis_limit_'+i]-.02)*100)).toFixed(1));
    document.querySelectorAll(`[data-setting="axis_limit_${i}"][type=range]`).forEach(el=>el.min=Math.max(5,(c['axis_deadzone_'+i]+.02)*100).toFixed(1));
  }
}
function sourceValue(s,source){
  if(!s?.connected||source<0)return 0;
  if(source<6)return [s.raw.steering,s.raw.ly,s.raw.rx,s.raw.ry,s.raw.throttle,s.raw.brake][source];
  if(source<22)return (s.raw.buttons&(1<<(source-6)))?1:0;
  return s.raw.axes[source-22]||0;
}
function axisInput(s,c,i){
  const source=c['axis_source_'+i],negative=i<4?c['axis_negative_'+i]:-1;
  if(!s?.connected||source<0)return 0;
  let x=sourceValue(s,source)-(negative>=0?sourceValue(s,negative):0);
  if(c['axis_invert_'+i])x=i>=4&&negative<0&&source>=4&&source<22?1-x:-x;
  const center=c['axis_center_'+i];x-=center;x/=x<0?1+center:1-center;
  return Math.max(i<4?-1:0,Math.min(1,x));
}
function axisResponse(value,c,i){
  const magnitude=Math.abs(value),dz=c['axis_deadzone_'+i],limit=c['axis_limit_'+i];
  if(magnitude<=dz)return 0;
  return Math.sign(value)*Math.pow(Math.min(1,(magnitude-dz)/(limit-dz)),c['axis_gamma_'+i]);
}
function outputValues(s){return [s.report.lx/(s.report.lx<0?32768:32767),s.report.ly/(s.report.ly<0?32768:32767),s.report.rx/(s.report.rx<0?32768:32767),s.report.ry/(s.report.ry<0?32768:32767),s.report.rt/255,s.report.lt/255];}
function renderStudio(s){
  const c=studioConfig(),out=outputValues(s);
  for(let i=0;i<6;i++){
    const raw=axisInput(s,c,i),inside=Math.abs(raw)<=c['axis_deadzone_'+i];
    $('cal-in-'+i).textContent=(raw*100).toFixed(1)+'%';$('cal-out-'+i).textContent=(out[i]*100).toFixed(1)+'%';
    const gate=$('cal-gate-'+i);gate.textContent=!s.connected?'Sem entrada':c['axis_source_'+i]<0?'Desativado':inside?'Ignorado · zona morta':'Movimento enviado';gate.dataset.active=String(s.connected&&!inside);
    const route=document.querySelector(`[data-route-axis="${i}"]`);route.querySelector('.route-meter i').style.width=Math.abs(out[i])*100+'%';route.querySelector('.route-meter span').textContent=(out[i]*100).toFixed(0)+'%';
  }
  document.querySelectorAll('[data-output-bit]').forEach(el=>el.classList.toggle('active',Boolean(s.report.buttons&(1<<Number(el.dataset.outputBit)))));
  updateLearn(s);updateMeasure(s);if(page==='calibration')drawCalibration();
}
async function startLearn(key,button){
  if(!state?.connected)return toast('Conecte o volante ou use a demonstração.');
  if(studio.learn){studio.learn.button.textContent='Identificar';if(studio.learn.key===key){studio.learn=null;return;}}
  studio.learn=null;button.textContent='Preparando…';
  try{
    const response=await fetch('/api/state',{cache:'no-store',signal:AbortSignal.timeout(1800)});
    if(!response.ok)throw new Error();const latest=await response.json();
    studio.learn={key,button,baseline:studio.sources.map(i=>sourceValue(latest,i)),armed:performance.now()+100,end:performance.now()+10000};
    button.textContent='Aguardando…';toast('Mova apenas a entrada desejada, ou pressione um botão.');
  }catch{button.textContent='Identificar';toast('Não foi possível ler a entrada. Tente novamente.');}
}
function updateLearn(s){
  const learn=studio.learn;if(!learn)return;
  if(!s.connected||performance.now()>learn.end){learn.button.textContent='Identificar';studio.learn=null;toast('Identificação encerrada. Tente novamente.');return;}
  if(performance.now()<learn.armed)return;
  let best=.3,source=-1,sign=1;
  studio.sources.forEach((value,index)=>{if(value>=6&&value<22&&sourceValue(s,value)<.5)return;const delta=sourceValue(s,value)-learn.baseline[index];const score=Math.abs(delta);if(score>best+.001){best=score;source=value;sign=delta<0?-1:1;}});
  if(source<0)return;
  const patch={[learn.key]:source};
  if(learn.key.startsWith('button_source_'))patch[learn.key.replace('source','direction')]=source>=4&&source<22?1:sign;
  learn.button.textContent='Identificar';studio.learn=null;queueSettings(patch);toast('Entrada identificada: '+sourceName(source));
}
function startMeasure(axis,button){
  if(!state?.connected)return toast('Conecte uma entrada antes de medir.');
  if(studio.measure)studio.measure.button.textContent='Medir repouso';
  studio.measure={axis,button,peak:0,count:0,end:performance.now()+2500};toast('Solte o controle e mantenha em repouso por 3 segundos.');
}
function updateMeasure(s){
  const m=studio.measure;if(!m)return;
  if(!s.connected){m.button.textContent='Medir repouso';studio.measure=null;return toast('Medição cancelada: entrada desconectada.');}
  m.peak=Math.max(m.peak,Math.abs(axisInput(s,studioConfig(),m.axis)));m.count++;
  m.button.textContent='Medindo… '+Math.max(1,Math.ceil((m.end-performance.now())/1000))+'s';
  if(performance.now()<m.end)return;
  m.button.textContent='Medir repouso';studio.measure=null;
  if(m.peak>.3||m.count<10)return toast('A entrada se moveu demais ou faltaram amostras. Repita com o controle solto.');
  const limit=studioConfig()['axis_limit_'+m.axis];queueSettings({['axis_deadzone_'+m.axis]:Math.min(limit-.02,m.peak+.005)});toast('Zona morta ajustada ao repouso medido, com uma pequena margem.');
}
function bindCanvas(canvas,i){
  let drag=null;
  function position(event){
    const rect=canvas.getBoundingClientRect(),x=event.clientX-rect.left,y=event.clientY-rect.top;
    if(i>=4)return Math.max(0,Math.min(1,(150-y)/125));
    return Math.max(0,Math.min(1,Math.abs(Math.atan2(x-rect.width/2,145-y))/(Math.PI/2)));
  }
  function move(event){
    if(!drag)return;const c=studioConfig();let v=position(event),key=drag;
    if(key.includes('deadzone'))v=Math.max(0,Math.min(.6,c['axis_limit_'+i]-.02,v));
    else v=Math.max(.05,c['axis_deadzone_'+i]+.02,Math.min(1,v));
    studio.draft[key]=Number(v.toFixed(4));syncStudio(state.config);drawCalibration();scheduleStudioSave();
  }
  canvas.addEventListener('pointerdown',event=>{
    if(!state)return;const v=position(event),c=studioConfig();
    drag=Math.abs(v-c['axis_deadzone_'+i])<=Math.abs(v-c['axis_limit_'+i])?'axis_deadzone_'+i:'axis_limit_'+i;
    canvas.setPointerCapture(event.pointerId);move(event);
  });
  canvas.addEventListener('pointermove',move);
  const finish=()=>{if(drag){drag=null;flushStudioSave();}};
  canvas.addEventListener('pointerup',finish);canvas.addEventListener('pointercancel',finish);
}
function drawCalibration(){
  if(!state||!studio)return;const c=studioConfig(),out=outputValues(state),dark=document.documentElement.dataset.theme==='dark';
  const colors={base:dark?'#344d53':'#d9e7e1',text:dark?'#bed5d5':'#52736d',raw:dark?'#c2cedd':'#718798',output:dark?'#6ce2ba':'#19896d',dead:'#dd9c46',limit:dark?'#7fbfff':'#487bbd',grid:dark?'#385157':'#d1e1d9'};
  for(let i=0;i<6;i++){
    const data=setupCanvas('cal-canvas-'+i);if(!data)continue;
    const {ctx,w}=data,raw=axisInput(state,c,i),dz=c['axis_deadzone_'+i],limit=c['axis_limit_'+i];
    ctx.lineCap='round';
    function line(x,y,xx,yy,color,width=2){ctx.beginPath();ctx.moveTo(x,y);ctx.lineTo(xx,yy);ctx.strokeStyle=color;ctx.lineWidth=width;ctx.stroke();}
    function dot(x,y,r,color){ctx.beginPath();ctx.arc(x,y,r,0,Math.PI*2);ctx.fillStyle=color;ctx.fill();}
    if(i<4){
      const cx=w/2,cy=145,r=Math.min(110,w*.43),point=(v,rad=r)=>[cx+Math.sin(v*Math.PI/2)*rad,cy-Math.cos(v*Math.PI/2)*rad];
      function arc(from,to,color,width){ctx.beginPath();ctx.arc(cx,cy,r,Math.PI*1.5+from*Math.PI/2,Math.PI*1.5+to*Math.PI/2);ctx.strokeStyle=color;ctx.lineWidth=width;ctx.stroke();}
      arc(-1,1,colors.base,16);arc(-dz,dz,colors.dead,16);
      for(const sign of [-1,1]){const [dx,dy]=point(dz*sign),[lx,ly]=point(limit*sign);dot(dx,dy,6,colors.dead);dot(lx,ly,6,colors.limit);}
      const [ix,iy]=point(raw,r-9),[ox,oy]=point(out[i],r-27);
      line(cx,cy,ix,iy,colors.raw,2);dot(ix,iy,5,colors.raw);line(cx,cy,ox,oy,colors.output,4);dot(ox,oy,5,colors.output);dot(cx,cy,6,colors.output);
      ctx.fillStyle=colors.text;ctx.font='11px sans-serif';ctx.textAlign='center';ctx.fillText('−100%',cx-r,165);ctx.fillText('0',cx,165);ctx.fillText('+100%',cx+r,165);
    }else{
      const x=w/2-37,top=25,height=125,width=74;
      ctx.fillStyle=colors.base;ctx.fillRect(x,top,width,height);
      ctx.fillStyle=colors.raw;ctx.fillRect(x+6,top+height-raw*height,25,raw*height);
      ctx.fillStyle=colors.output;ctx.fillRect(x+43,top+height-out[i]*height,25,out[i]*height);
      ctx.fillStyle=dark?'#dd9c4655':'#dd9c4644';ctx.fillRect(x,top+height-dz*height,width,dz*height);
      for(const [v,color] of [[dz,colors.dead],[limit,colors.limit]]){const y=top+height-v*height;line(x-18,y,x+width+18,y,color);dot(x+width+18,y,6,color);}
      ctx.fillStyle=colors.text;ctx.font='11px sans-serif';ctx.textAlign='center';ctx.fillText('Entrada',x+17,167);ctx.fillText('Saída',x+57,167);
    }
    // Response curve is drawn from the same normalized function as the C mapper.
    const left=23,top=190,right=w-23,bottom=242;
    for(let n=0;n<=4;n++){const x=left+(right-left)*n/4,y=bottom-(bottom-top)*n/4;line(x,top,x,bottom,colors.grid,1);line(left,y,right,y,colors.grid,1);}
    ctx.beginPath();
    for(let n=0;n<=100;n++){const x=left+(right-left)*n/100,y=bottom-axisResponse(n/100,c,i)*(bottom-top);n?ctx.lineTo(x,y):ctx.moveTo(x,y);}
    ctx.strokeStyle=colors.output;ctx.lineWidth=2;ctx.stroke();
    dot(left+Math.abs(raw)*(right-left),bottom-Math.abs(out[i])*(bottom-top),4,colors.raw);
    ctx.fillStyle=colors.text;ctx.font='10px sans-serif';ctx.textAlign='center';ctx.fillText('movimento → resposta',w/2,258);
  }
}
function setupTheme(){
  let theme;try{theme=localStorage.getItem('boreal-wheel-theme')||localStorage.getItem('project-b-theme');}catch{}
  if(!['dark','light'].includes(theme))theme=matchMedia('(prefers-color-scheme: dark)').matches?'dark':'light';
  function apply(value){document.documentElement.dataset.theme=value;$('theme-toggle').textContent=value==='dark'?'Tema claro':'Tema escuro';$('theme-toggle').setAttribute('aria-pressed',String(value==='dark'));document.querySelector('meta[name="theme-color"]').content=value==='dark'?'#14252b':'#d9f1ed';drawCharts();}
  apply(theme);$('theme-toggle').onclick=()=>{const next=document.documentElement.dataset.theme==='dark'?'light':'dark';apply(next);try{localStorage.setItem('boreal-wheel-theme',next);}catch{}};
}
function setupProfiles(){
  const feedback=(text,error=false)=>{$('profile-feedback').textContent=text;$('profile-feedback').dataset.error=String(error);};
  async function profile(){await flushStudioSave();await configQueue;const response=await fetch('/api/profile',{cache:'no-store'});if(!response.ok)throw new Error('Não foi possível ler o perfil.');return response.json();}
  function encode(value){return 'BW2.'+btoa(JSON.stringify(value));}
  function validate(value){
    if(!value||!['boreal-wheel','project-b'].includes(value.format)||value.version!==2||!value.settings||Array.isArray(value.settings)||typeof value.settings!=='object')throw new Error('Este perfil não é um perfil Boreal Wheel versão 2.');
    const required=Object.keys(state.config).filter(key=>/^(axis_|button_(source|direction|threshold)_)/.test(key)||['poll_hz','slot','directinput','steering_axis','pedal_axis'].includes(key));
    const keys=Object.keys(value.settings);
    if(keys.length!==required.length||keys.some(key=>!required.includes(key))||required.some(key=>!Object.hasOwn(value.settings,key)))throw new Error('O perfil está incompleto ou contém campos desconhecidos.');
    if(Object.values(value.settings).some(value=>typeof value!=='number'||!Number.isFinite(value)))throw new Error('O perfil contém valores inválidos.');
    return value;
  }
  async function apply(text){
    if(!state||!online)throw new Error('Abra o Boreal Wheel antes de importar.');
    if(text.length>40000)throw new Error('Código ou arquivo grande demais.');
    let value;try{const trimmed=text.trim();value=JSON.parse(/^(BW2|PB2)\./.test(trimmed)?atob(trimmed.slice(4)):trimmed);}catch{throw new Error('Código ou arquivo inválido. Cole o código completo, começando por BW2.');}
    validate(value);await flushStudioSave();await configQueue;
    // One request validates and saves the complete profile before replacing it.
    await post('profile',value.settings);studio.draft={};configSignature='';await refresh();
    feedback('Perfil aplicado e salvo. A ponte mantém o estado atual; ative-a se estiver desligada.');toast('Perfil importado.');
  }
  $('profile-export').onclick=async()=>{try{$('profile-code').value=encode(await profile());feedback('Código gerado. Use Copiar código para guardar ou compartilhar.');}catch(error){feedback(error.message,true);}};
  $('profile-copy').onclick=async()=>{try{if(!$('profile-code').value)$('profile-code').value=encode(await profile());try{await navigator.clipboard.writeText($('profile-code').value);}catch{$('profile-code').focus();$('profile-code').select();if(!document.execCommand('copy'))throw new Error('Selecione o código e pressione Ctrl+C.');}feedback('Código copiado.');}catch(error){feedback(error.message,true);}};
  $('profile-import').onclick=async()=>{try{await apply($('profile-code').value);}catch(error){feedback('Não foi possível importar: '+error.message,true);}};
  $('profile-download').onclick=async()=>{try{const value=await profile(),url=URL.createObjectURL(new Blob([JSON.stringify(value,null,2)],{type:'application/json'}));const link=document.createElement('a');link.href=url;link.download='boreal-wheel-perfil.json';link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);feedback('Perfil exportado em JSON.');}catch(error){feedback(error.message,true);}};
  $('profile-file-open').onclick=()=>$('profile-file').click();
  $('profile-file').onchange=async event=>{try{const file=event.target.files[0];if(!file)return;if(file.size>40000)throw new Error('Arquivo grande demais.');await apply(await file.text());}catch(error){feedback('Não foi possível importar: '+error.message,true);}finally{event.target.value='';}};
}
