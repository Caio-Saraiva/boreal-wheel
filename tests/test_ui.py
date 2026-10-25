#!/usr/bin/env python3
"""Real Firefox + native C integration: mapping, calibration, profiles and themes."""
import base64
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request
ROOT=Path(__file__).resolve().parents[1]
def free_port():
    with socket.socket() as sock:sock.bind(('127.0.0.1',0));return sock.getsockname()[1]
def request(url,data=None,method=None):
    with urllib.request.urlopen(urllib.request.Request(url,data=json.dumps(data).encode() if data is not None else None,headers={'Content-Type':'application/json'},method=method),timeout=30) as response:return json.load(response)['value']
def run(language="en"):
    driver_port,core_port=free_port(),free_port();driver_base=f'http://127.0.0.1:{driver_port}';core_base=f'http://127.0.0.1:{core_port}';session=None
    def api(path='state',values=None):
        req=urllib.request.Request(core_base+'/api/'+path,data=urllib.parse.urlencode(values).encode() if values is not None else None,headers={'Content-Type':'application/x-www-form-urlencoded','Origin':core_base})
        with urllib.request.urlopen(req,timeout=3) as response:return json.load(response)
    def wait_api(predicate):
        for _ in range(150):
            state=api()
            if predicate(state):return state
            time.sleep(.03)
        raise AssertionError('Native state did not reach expected result: '+json.dumps(state))
    with tempfile.TemporaryDirectory(prefix='boreal-wheel-ui-') as directory,open(ROOT/'build/ui-test.log','w') as log:
        shutil.copytree(ROOT/'web',Path(directory)/'web')
        core=subprocess.Popen([str(ROOT/'build/boreal-wheel'),'--demo','--lang',language,'--port',str(core_port)],cwd=directory,stdout=log,stderr=log)
        driver=subprocess.Popen([os.environ.get('GECKODRIVER','geckodriver'),'--port',str(driver_port),'--host','127.0.0.1'],stdout=log,stderr=log)
        try:
            for _ in range(100):
                try:request(driver_base+'/status');break
                except OSError:time.sleep(.05)
            options={'args':['-headless'],'prefs':{'ui.prefersReducedMotion':1,'intl.accept_languages':'pt-BR,pt'}}
            if os.environ.get('FIREFOX_BINARY'):options['binary']=os.environ['FIREFOX_BINARY']
            session=request(driver_base+'/session',{'capabilities':{'alwaysMatch':{'browserName':'firefox','moz:firefoxOptions':options}}})['sessionId'];base=driver_base+'/session/'+session
            def cmd(path,data=None):return request(base+path,data)
            def js(script,*args):return cmd('/execute/sync',{'script':script,'args':list(args)})
            def wait(script):
                for _ in range(150):
                    if js(script):return
                    time.sleep(.04)
                raise AssertionError('Timed out: '+script)
            def click(selector):
                element=cmd('/element',{'using':'css selector','value':selector});cmd('/element/'+element['element-6066-11e4-a52e-4f735466cecf']+'/click',{})
            def setting(key,value):
                js('const el=document.querySelector(`[data-setting="${arguments[0]}"][type=number]`)||document.querySelector(`[data-setting="${arguments[0]}"]`);el.value=arguments[1];el.dispatchEvent(new Event("change",{bubbles:true}));',key,str(value))
            def screenshot(name):
                (ROOT/'docs'/(('pt-BR/images/'+name) if language=='pt-BR' else name)).write_bytes(base64.b64decode(cmd('/screenshot')))
            cmd('/window/rect',{'width':1440,'height':1120});cmd('/url',{'url':core_base})
            wait('return document.querySelector("#device-badge").textContent.includes("Demonstration")')
            assert js('return document.title').startswith('Boreal Wheel')
            assert js('return document.documentElement.lang')=='en', 'English must be the default even for a Portuguese browser'
            assert js('return Object.entries(BW_MESSAGES).filter(([en])=>!en.includes("{")).every(([en,pt])=>BWI18n.t(en,"pt-BR")===pt && BWI18n.t(pt,"en")===en)')
            assert js('return BWI18n.t("Demonstração automática","en")')=='Demonstration automatic'
            assert js('return BWI18n.t("Xbox 360 virtual ativo · slot 2","en")')=='Virtual Xbox 360 active · slot 2'
            assert js('return BWI18n.t("Virtual Xbox 360 active · slot 2","pt-BR")')=='Xbox 360 virtual ativo · slot 2'

            def choose_language(value):
                js('const el=document.querySelector("#language-select");el.value=arguments[0];el.dispatchEvent(new Event("change",{bubbles:true}));',value)
                wait('return document.documentElement.lang==='+json.dumps(value))
            before_language=api('profile')
            js('document.querySelector("#profile-code").value="BW2.keep-this-user-input"')
            choose_language('pt-BR')
            assert js('return document.querySelector("[data-page=mapping]").textContent')=='Mapeamento'
            assert js('return document.querySelector("#export-diagnostics").getAttribute("aria-label")')=='Exportar diagnóstico'
            assert js('return document.querySelector("#language-select").getAttribute("aria-label")')=='Idioma'
            assert js('return document.querySelector("#diagnostic-detail").textContent').startswith('Entradas sintéticas')
            assert js('return document.querySelector("#profile-code").value')=='BW2.keep-this-user-input'
            assert api('profile')==before_language
            choose_language('en')
            assert js('return document.querySelector("[data-page=mapping]").textContent')=='Mapping'
            choose_language(language)
            words={'demo':'Demonstration','waiting':'Waiting','sent':'sent','applied':'applied'} if language=='en' else {'demo':'Demonstração','waiting':'Aguardando','sent':'enviado','applied':'aplicado'}

            # The rename preserves the browser preference from older installations.
            js('localStorage.removeItem("boreal-wheel-theme");localStorage.setItem("project-b-theme","dark")')
            cmd('/refresh',{});wait('return document.querySelector("#device-badge").textContent.includes('+json.dumps(words["demo"])+')')
            assert js('return document.documentElement.lang')==language
            assert js('return document.documentElement.dataset.theme')=='dark'
            click('#theme-toggle')
            assert js('return document.querySelectorAll(".mapping-row").length')==14
            assert js('return document.querySelectorAll(".route-card").length')==6
            assert js('return document.querySelectorAll(".calibration-card").length')==6
            assert js('return document.querySelector("#bridge-toggle").disabled')
            click('#demo-interaction');wait('return !document.querySelector("#manual-controls").hidden')
            cmd('/actions',{'actions':[{'type':'key','id':'keyboard','actions':[{'type':'keyDown','value':'w'},{'type':'keyDown','value':'d'},{'type':'keyDown','value':'j'}]}]})
            wait('return document.querySelector("#rt-value").textContent==="255" && document.querySelector("#lx-small").textContent==="32767"')
            cmd('/actions',{'actions':[{'type':'key','id':'keyboard','actions':[{'type':'keyUp','value':'w'},{'type':'keyUp','value':'d'},{'type':'keyUp','value':'j'}]}]})
            wait_api(lambda s: not s['report']['rt'])
            click('[data-page="mapping"]');click('#page-mapping .swap-pedals')
            wait_api(lambda s:s['config']['axis_source_4']==5 and s['config']['axis_source_5']==4)
            api('demo',{'brake':1});wait('return document.querySelector("#rt-value").textContent==="255" && document.querySelector("#lt-value").textContent==="000"')
            assert js('return document.querySelector("#throttle-value").textContent')=='100%'
            # Analog-to-button routing and learn-by-moving use real browser events.
            setting('button_source_12',2);setting('button_direction_12',-1);setting('button_threshold_12',60)
            api('demo',{'rx':-.7});wait_api(lambda s:s['report']['buttons']==4096)
            wait('return document.querySelector("[data-output-bit=\\"12\\"]").classList.contains("active")')
            setting('button_source_12',18);wait_api(lambda s:s['config']['button_source_12']==18 and s['config']['button_direction_12']==1)
            setting('button_source_12',2);setting('button_direction_12',-1)
            api('demo',{});wait_api(lambda s:s['raw']['rx']==0)
            click('#source-axis_source_2 + button');wait('return document.querySelector("#source-axis_source_2 + button").textContent.includes('+json.dumps(words["waiting"])+')');time.sleep(.12);api('demo',{'ry':.8});wait_api(lambda s:s['config']['axis_source_2']==3)
            # Digital pair controls both directions of a stick.
            setting('axis_source_0',15);setting('axis_negative_0',14)
            api('demo',{'buttons':256});wait_api(lambda s:s['report']['lx']==-32768)
            api('demo',{'buttons':512});wait_api(lambda s:s['report']['lx']==32767)
            setting('axis_source_0',0);setting('axis_negative_0',-1)
            click('[data-page="calibration"]');setting('axis_deadzone_0',10);setting('axis_limit_0',50)
            api('demo',{'steering':.3,'brake':.5});s=wait_api(lambda s:s['config']['axis_limit_0']==.5 and s['config']['axis_deadzone_0']==.1 and abs(s['report']['lx']/32767-.5)<.001)
            assert abs(s['report']['lx']/32767-.5)<.001
            wait('return document.querySelector("#cal-gate-0").textContent.includes('+json.dumps(words["sent"])+')')
            # Drag the actual moon deadzone handle using WebDriver pointer input.
            js('document.querySelector("#cal-canvas-0").scrollIntoView({block:"center"})')
            points=js('const r=document.querySelector("#cal-canvas-0").getBoundingClientRect(),radius=Math.min(110,r.width*.43);return [.1,.14,.18].map(v=>({x:Math.round(r.left+r.width/2+Math.sin(v*Math.PI/2)*radius),y:Math.round(r.top+145-Math.cos(v*Math.PI/2)*radius)}));')
            # A delayed save response must not discard a newer unsaved drag position.
            js('window.bwNativeFetch=window.fetch;window.fetch=async(...args)=>{const response=await window.bwNativeFetch(...args);if(String(args[0]).endsWith("/api/config"))await new Promise(resolve=>setTimeout(resolve,65));return response;}')
            cmd('/actions',{'actions':[{'type':'pointer','id':'mouse','parameters':{'pointerType':'mouse'},'actions':[{'type':'pointerMove','duration':0,'x':points[0]['x'],'y':points[0]['y']},{'type':'pointerDown','button':0},{'type':'pointerMove','duration':0,'x':points[1]['x'],'y':points[1]['y']}]}]})
            time.sleep(.13)
            cmd('/actions',{'actions':[{'type':'pointer','id':'mouse','parameters':{'pointerType':'mouse'},'actions':[{'type':'pointerMove','duration':0,'x':points[2]['x'],'y':points[2]['y']}]}]})
            time.sleep(.08)
            cmd('/actions',{'actions':[{'type':'pointer','id':'mouse','parameters':{'pointerType':'mouse'},'actions':[{'type':'pointerUp','button':0}]}]})
            s=wait_api(lambda s:.16<s['config']['axis_deadzone_0']<.2 and abs(s['report']['lx']/32767-(.3-s['config']['axis_deadzone_0'])/(.5-s['config']['axis_deadzone_0']))<.001)
            js('window.fetch=window.bwNativeFetch')
            expected=(.3-s['config']['axis_deadzone_0'])/(.5-s['config']['axis_deadzone_0'])
            assert abs(s['report']['lx']/32767-expected)<.001
            # Vertical pedal deadzone handle, independent of the steering setting.
            js('document.querySelector("#cal-canvas-4").scrollIntoView({block:"center"})')
            points=js('const r=document.querySelector("#cal-canvas-4").getBoundingClientRect();return [.01,.2].map(v=>({x:Math.round(r.left+r.width/2+55),y:Math.round(r.top+150-v*125)}));')
            cmd('/actions',{'actions':[{'type':'pointer','id':'mouse','parameters':{'pointerType':'mouse'},'actions':[{'type':'pointerMove','duration':0,'x':points[0]['x'],'y':points[0]['y']},{'type':'pointerDown','button':0},{'type':'pointerMove','duration':200,'x':points[1]['x'],'y':points[1]['y']},{'type':'pointerUp','button':0}]}]})
            wait_api(lambda s:.19<s['config']['axis_deadzone_4']<.21)
            api('demo',{'steering':.12});wait_api(lambda s:s['raw']['steering']==.12)
            click('#calibrate-center');wait_api(lambda s:abs(s['config']['axis_center_0']-.12)<.001 and s['report']['lx']==0)
            click('[data-measure="0"]');wait_api(lambda s:s['config']['axis_deadzone_0']<.01)
            # Dark mode persists through a full reload.
            js('document.querySelector("#theme-toggle").scrollIntoView({block:"center"})')
            if js('return document.documentElement.dataset.theme')!='dark':click('#theme-toggle')
            assert js('return getComputedStyle(document.body).backgroundColor')=='rgb(17, 36, 42)'
            api('demo',{'steering':.3,'brake':.68,'throttle':.12});wait_api(lambda s:s['raw']['steering']==.3);time.sleep(.08)
            js('window.scrollTo(0,0)');screenshot('calibration-dark.png')
            cmd('/refresh',{});wait('return document.querySelector("#device-badge").textContent.includes('+json.dumps(words["demo"])+')')
            assert js('return document.documentElement.lang')==language
            assert js('return document.documentElement.dataset.theme')=='dark'
            # Export code, reject corruption without touching settings, import roundtrip.
            click('[data-page="mapping"]');click('#profile-export')
            wait('return document.querySelector("#profile-code").value.startsWith("BW2.")')
            code=js('return document.querySelector("#profile-code").value');profile=api('profile')
            assert json.loads(base64.b64decode(code[4:]))==profile
            setting('axis_source_4',4);wait_api(lambda s:s['config']['axis_source_4']==4)
            before=api('profile');js('document.querySelector("#profile-code").value="BW2.invalid"');click('#profile-import')
            wait('return document.querySelector("#profile-feedback").dataset.error==="true"')
            assert api('profile')==before
            js('document.querySelector("#profile-code").value=arguments[0]',code);click('#profile-import')
            wait('return document.querySelector("#profile-feedback").textContent.includes('+json.dumps(words["applied"])+')');assert api('profile')==profile
            # Existing PB2 codes and JSON files remain importable after the rename.
            legacy={**profile,'format':'project-b'}
            legacy_code='PB2.'+base64.b64encode(json.dumps(legacy).encode()).decode()
            setting('axis_source_4',4);wait_api(lambda s:s['config']['axis_source_4']==4)
            js('document.querySelector("#profile-code").value=arguments[0]',legacy_code);click('#profile-import')
            wait_api(lambda s:s['config']['axis_source_4']==5)
            assert api('profile')==profile
            incompatible={**profile,'version':999}
            js('document.querySelector("#profile-code").value=arguments[0]',json.dumps(incompatible));click('#profile-import')
            wait('return document.querySelector("#profile-feedback").dataset.error==="true"')
            assert api('profile')==profile
            # JSON file import uses the same complete, atomic profile endpoint.
            setting('axis_source_4',4);wait_api(lambda s:s['config']['axis_source_4']==4)
            js('const input=document.querySelector("#profile-file"),transfer=new DataTransfer();transfer.items.add(new File([arguments[0]],"perfil.json",{type:"application/json"}));input.files=transfer.files;input.dispatchEvent(new Event("change",{bubbles:true}));',json.dumps(legacy))
            wait_api(lambda s:s['config']['axis_source_4']==5)
            click('[data-page="calibration"]');js('window.scrollTo(0,0)');click('#theme-toggle');screenshot('calibration-light.png')
            click('[data-page="diagnostics"]');click('#help-top');assert js('return document.querySelector("#help-dialog").open');click('#help-done')
            # Predictable screenshots and responsive checks on all major pages.
            api('action',{'action':'reset'});api('demo',{'steering':.18,'throttle':.68,'brake':.12,'buttons':4096})
            click('[data-page="overview"]');wait('return document.querySelector("#rt-value").textContent==="173"')
            js('window.scrollTo(0,0)');screenshot('preview-desktop.png')
            for width in [1440,390]:
                cmd('/window/rect',{'width':width,'height':1120})
                for page in ['overview','mapping','calibration','diagnostics']:
                    click('[data-page="'+page+'"]');assert not js('return document.documentElement.scrollWidth>innerWidth'),(page,width)
                if width==390:
                    click('[data-page="calibration"]');js('window.scrollTo(0,0)');screenshot('calibration-mobile.png')
            click('[data-page="overview"]');js('window.scrollTo(0,0)');screenshot('preview-mobile.png')
            click('#physical-mode');wait('return document.querySelector("#device-badge").textContent.includes('+json.dumps(words["waiting"])+')')
            core.terminate();core.wait(timeout=5);wait('return !document.querySelector("#connection-error").hidden');assert js('return document.querySelector("#bridge-toggle").disabled')
            print(language+': Firefox: all sources/outputs, pedal swap, axis-to-button threshold, input learning, digital pairs, numeric calibration, moon/bar drags, measured deadzone, center capture, profile/code/file roundtrip, invalid import atomicity, persistent dark theme, responsive layout and offline state passed.')
        finally:
            if session:
                try:request(driver_base+'/session/'+session,method='DELETE')
                except OSError:pass
            for process in [core,driver]:
                if process.poll() is None:process.terminate()
                try:process.wait(timeout=5)
                except subprocess.TimeoutExpired:process.kill()
if __name__=='__main__':
    for language in ['en','pt-BR']:run(language)
