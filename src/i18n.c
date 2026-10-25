#include "bridge.h"
#include <string.h>
static int portuguese;
int bw_set_language(const char *language) {
    if(!language || !strcmp(language,"en")){portuguese=0;return 1;}
    if(!strcmp(language,"pt-BR")){portuguese=1;return 1;}
    return 0;
}
const char *bw_text(const char *english) {
    static const struct {const char *en,*pt;} messages[]={
    {"\n  BOREAL WHEEL / Input, reconnected.\n  http://127.0.0.1:%d\n  Mode: %s\n  Ctrl+C to stop.\n\n","\n  BOREAL WHEEL / Input, reconnected.\n  http://127.0.0.1:%d\n  Modo: %s\n  Ctrl+C para encerrar.\n\n"},
    {"Cannot save profile","Não foi possível salvar o perfil"},
    {"Cannot save profile.ini","Não foi possível salvar profile.ini"},
    {"Chunking unsupported","Transferência em blocos não suportada"},
    {"Combined pedals in DirectInput: simultaneous throttle + brake are not preserved. Prefer XInput or select the physical slot.","Pedais combinados em DirectInput: não preserva acelerador + freio simultâneos. Prefira XInput ou selecione o slot físico."},
    {"Connect the Ferrari 458 Italia Xbox 360 by USB. Linux input requires the xpad module.","Conecte o Ferrari 458 Italia Xbox 360 por USB. A leitura Linux requer o módulo xpad."},
    {"Could not create a virtual Xbox 360 controller in ViGEmBus","Falha ao criar Xbox 360 virtual no ViGEmBus"},
    {"Could not safely identify the virtual slot","Não foi possível identificar o slot virtual com segurança"},
    {"Could not send report to ViGEmBus","Falha ao enviar relatório ao ViGEmBus"},
    {"DEMONSTRATION (no hardware detection)","DEMONSTRAÇÃO (sem detecção de hardware)"},
    {"Demo cannot feed games","A demonstração não pode alimentar jogos"},
    {"Demonstration %s","Demonstração %s"},
    {"Demonstration · game output blocked","Demonstração · saída para jogos bloqueada"},
    {"Ferrari 458 Italia · simulation","Ferrari 458 Italia · simulação"},
    {"Hold the control at full travel before capturing","Mantenha o controle no fim do curso antes de capturar"},
    {"Incompatible ViGEmClient.dll · check the x64 build","ViGEmClient.dll incompatível · confira build x64"},
    {"Independent axes · LX steering / RT throttle / LT brake. Confirm the device by moving its controls.","Eixos independentes · LX direção / RT acelerador / LT freio. Confirme o modelo pelos movimentos."},
    {"Invalid demo input","Entrada de demonstração inválida"},
    {"Invalid executable path","Caminho do executável inválido"},
    {"Invalid header","Cabeçalho inválido"},
    {"Invalid language. Use en or pt-BR.\n","Idioma inválido. Use en ou pt-BR.\n"},
    {"Invalid length","Comprimento inválido"},
    {"Invalid port\n","Porta inválida\n"},
    {"Invalid request","Requisição inválida"},
    {"Invalid setting","Ajuste inválido"},
    {"Keep the control near center before capturing","Deixe o controle proximo do centro antes de capturar"},
    {"Linux preview · XInput output is only available on Windows","Prévia Linux · saída XInput disponível apenas no Windows"},
    {"No input available for calibration","Sem entrada para calibrar"},
    {"No readable input. Check the Xbox 360 driver in devmgmt.msc and test in joy.cpl. If listed as a gamepad, select the physical slot.","Sem entrada legível. Confira o driver Xbox 360 em devmgmt.msc e teste em joy.cpl. Se listado como gamepad, escolha o slot físico."},
    {"Not found","Não encontrado"},
    {"PHYSICAL INPUT","ENTRADA FÍSICA"},
    {"Port %d unavailable. Try --port 4581.\n","Porta %d indisponível. Tente --port 4581.\n"},
    {"Request too large","Requisição grande demais"},
    {"Same-origin form requests required","São necessárias requisições de formulário da mesma origem"},
    {"Synthetic inputs for interface testing. No hardware is detected in this mode; game output is blocked.","Entradas sintéticas para validar a interface. Nenhum hardware foi detectado neste modo; saída para jogos bloqueada."},
    {"The selected XInput slot is unavailable (or is Boreal Wheel's virtual output).","Slot XInput selecionado não disponível (ou é a saída virtual do Boreal Wheel)."},
    {"Thrustmaster Ferrari 458 Italia · USB detected","Thrustmaster Ferrari 458 Italia · USB detectado"},
    {"UI assets missing; start from boreal-wheel","Arquivos da interface ausentes; inicie na pasta boreal-wheel"},
    {"USB detected, but cannot read %s. Check Linux permissions in the README.","USB detectado, mas sem leitura de %s. Verifique permissões Linux no README."},
    {"Unknown action","Ação desconhecida"},
    {"Unsupported method","Método não suportado"},
    {"Usage: boreal-wheel [--demo] [--port 4580] [--lang en|pt-BR]\n","Uso: boreal-wheel [--demo] [--port 4580] [--lang en|pt-BR]\n"},
    {"Use the 127.0.0.1 address printed in the terminal","Use o endereço 127.0.0.1 exibido no terminal"},
    {"ViGEmBus unavailable · install the official driver","ViGEmBus não disponível · instale o driver oficial"},
    {"ViGEmClient.dll missing next to the executable","ViGEmClient.dll ausente ao lado do executável"},
    {"Virtual Xbox 360 active · slot %lu","Xbox 360 virtual ativo · slot %lu"},
    {"Virtual output disabled","Saída virtual desativada"},
    {"Waiting for physical input · output neutralized","Aguardando entrada física · saída neutralizada"},
    {"Waiting to retry ViGEmBus","Aguardando nova tentativa do ViGEmBus"},
    {"Wheel present on USB, but no readable input. Check the Xbox 360 driver in Device Manager and test in joy.cpl; try the physical XInput slot in Diagnostics.","Volante presente no USB, mas sem entrada legível. Confira o vínculo com o driver Xbox 360 no Gerenciador de Dispositivos e teste em joy.cpl; tente o slot XInput físico em Diagnóstico."},
    {"XInput controller · manual selection","Controle XInput · seleção manual"},
    {"XInput wheel (WHEEL subtype)","Volante XInput (subtipo WHEEL)"},
    {"automatic","automática"},
    {"interactive","interativa"},
    };
    if(portuguese)for(unsigned i=0;i<sizeof(messages)/sizeof(messages[0]);++i)
        if(!strcmp(english,messages[i].en))return messages[i].pt;
    return english;
}
