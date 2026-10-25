'use strict';
// Translate display text only. API keys, profile codes and user input stay unchanged.
window.BWI18n=(()=>{
  const messages=window.BW_MESSAGES;
  let language='en';
  try{if(localStorage.getItem('boreal-wheel-language')==='pt-BR')language='pt-BR';}catch{}
  const reverse=new Map(Object.entries(messages).map(([en,pt])=>[pt,en]));
  const escape=value=>value.replace(/[.*+?^${}()|[\]\\]/g,'\\$&');
  const templates=Object.entries(messages).filter(([en])=>en.includes('{')).map(([en,pt])=>{
    function compile(value){
      const names=[];let expression='',last=0;
      for(const match of value.matchAll(/\{(\w+)\}/g)){
        expression+=escape(value.slice(last,match.index))+'(.+?)';names.push(match[1]);last=match.index+match[0].length;
      }
      return {regex:new RegExp('^'+expression+escape(value.slice(last))+'$'),names};
    }
    return {en,pt,english:compile(en),portuguese:compile(pt)};
  }).sort((a,b)=>b.en.replace(/\{\w+\}/g,'').length-a.en.replace(/\{\w+\}/g,'').length);
  function translate(value,target=language,depth=0){
    if(typeof value!=='string'||!value||depth>5)return value;
    const canonical=reverse.get(value)||value;
    if(Object.hasOwn(messages,canonical))return target==='pt-BR'?messages[canonical]:canonical;
    for(const item of templates){
      const source=item.english.regex.test(value)?item.english:item.portuguese;
      const match=value.match(source.regex);if(!match)continue;
      const captures=Object.fromEntries(source.names.map((name,i)=>[name,translate(match[i+1],target,depth+1)]));
      return (target==='pt-BR'?item.pt:item.en).replace(/\{(\w+)\}/g,(_,name)=>captures[name]);
    }
    const trimmed=value.trim();
    if(trimmed!==value)return value.replace(trimmed,translate(trimmed,target,depth+1));
    return value;
  }
  function translateTree(root){
    const doc=root.ownerDocument||root;
    const walker=doc.createTreeWalker(root,NodeFilter.SHOW_TEXT);
    let node;
    while((node=walker.nextNode())){
      if(node.parentElement?.closest('script,style,textarea,[data-no-i18n]'))continue;
      const translated=translate(node.nodeValue);if(translated!==node.nodeValue)node.nodeValue=translated;
    }
    const elements=[...(root.querySelectorAll?.('*')||[])];if(root.nodeType===Node.ELEMENT_NODE)elements.unshift(root);
    for(const element of elements){
      if(element.closest('[data-no-i18n]'))continue;
      for(const name of ['aria-label','title','placeholder','label']){
        if(element.hasAttribute(name))element.setAttribute(name,translate(element.getAttribute(name)));
      }
    }
  }
  function html(value){const template=document.createElement('template');template.innerHTML=value;translateTree(template.content);return template.innerHTML;}
  function setLanguage(value){
    language=value==='pt-BR'?'pt-BR':'en';document.documentElement.lang=language;
    try{localStorage.setItem('boreal-wheel-language',language);}catch{}
  }
  document.documentElement.lang=language;
  return {get language(){return language;},t:translate,translateTree,html,setLanguage};
})();
