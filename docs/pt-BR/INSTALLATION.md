# Instalação no Windows: volante → Boreal Wheel → jogo

[English](../INSTALLATION.md) | **Português (Brasil)**

A interface inicia em inglês. Selecione **Language → Português (Brasil)** no topo para acompanhar os nomes usados neste guia.

[Voltar ao README](../../README.pt-BR.md) · [Configuração](CONFIGURATION.md) · [Problemas](TROUBLESHOOTING.md)

## O que cada parte faz

```text
Ferrari 458 Italia Xbox 360
  → driver Xbox 360 do Windows
  → Boreal Wheel (lê, remapeia e calibra)
  → ViGEmClient.dll → ViGEmBus
  → controle Xbox 360 virtual → jogo

HidHide: esconde o volante físico do jogo e permite sua leitura pelo Boreal Wheel.
```

**ViGEmBus** é o driver que cria a saída virtual. **ViGEmClient.dll** é a biblioteca usada pelo programa para conversar com esse driver. **HidHide** evita que o jogo receba simultaneamente o dispositivo original e o virtual. Instalar apenas HidHide não cria um controle virtual.

## Requisitos

- PC Windows x64; Windows 11 é o alvo do aplicativo. A release HidHide 1.5.230 informa suporte a Windows 10/11 Intel/AMD de 64 bits.
- Volante **Thrustmaster Ferrari 458 Italia para Xbox 360**, conectado por USB e legível pelo driver do Windows.
- Navegador atual com JavaScript e canvas. Não abra `web/index.html` diretamente: a interface depende do processo nativo.
- Permissão de administrador para instalar os drivers e reiniciar quando solicitado. A execução cotidiana do aplicativo não pede instalação de drivers.
- Um slot XInput disponível. Outros remapeadores também podem ocupar slots.

## 1. Validar o volante físico

1. Com o jogo fechado, conecte o volante e pedais. Inicialmente deixe a ocultação do HidHide desligada.
2. Pressione `Win+R`, execute `devmgmt.msc` e confira o dispositivo. Ausência total ou aviso de driver deve ser resolvido antes da ponte.
3. Execute `joy.cpl`, abra **Propriedades** do dispositivo físico e mova direção, pedais e botões.
4. Se não houver leitura, siga o [artigo oficial Thrustmaster sobre driver Xbox 360](https://support.thrustmaster.com/en/kb/101-en/). Não use driver de TX/458 Spider como substituto para esse modelo.

Ter USB detectado não significa ter entrada legível. O adaptador depende do driver físico e não contém um driver de kernel para substituir o Xbox 360 do Windows.

## 2. Instalar o ViGEmBus

1. Abra a [release oficial ViGEmBus Setup 1.22.0](https://github.com/nefarius/ViGEmBus/releases/tag/v1.22.0).
2. Em **Assets**, baixe o instalador disponibilizado pelo mantenedor, execute-o e termine a instalação. Reinicie se solicitado.
3. O [ViGEm está descontinuado](https://docs.nefarius.at/projects/ViGEm/End-of-Life/). A release 1.22.0 removeu o atualizador legado; para instalações antigas com alertas de atualização, siga a orientação oficial.
4. Não copie o instalador para a pasta do programa esperando que ele vire a DLL: são componentes diferentes.

Se aparecer **ViGEmBus não disponível**, o driver ainda não ficou acessível. Confira a instalação/reinicialização antes de trocar configurações do mapper.

## 3. Extrair e iniciar o Boreal Wheel

Baixe o ZIP em [Releases](https://github.com/Caio-Saraiva/boreal-wheel/releases/latest). Extraia para uma pasta fixa em que você possa gravar, como `C:\BorealWheel`. Não execute de dentro do ZIP.

```text
C:\BorealWheel\
  boreal-wheel.exe
  ViGEmClient.dll
  run.cmd
  run-demo.cmd
  web\
  docs\
  README.md, LEIAME.md, LICENSE, licenças e SHA256SUMS.txt
```

1. Execute `run.cmd`. O terminal precisa permanecer aberto.
2. Acesse `http://127.0.0.1:4580`. Se abrir antes de o núcleo ficar pronto, recarregue a página.
3. Selecione **Dispositivo**. **Demo** é uma simulação e não envia entrada ao jogo.
4. Confirme **Entrada detectada** e mova os controles.
5. Caso o volante seja exposto como gamepad, experimente **Diagnóstico → Entrada XInput física**: slots 0 a 3. Faça isso com a ponte desligada para reconhecer o dispositivo original. O modo automático procura subtipo volante e depois tenta DirectInput para os modelos conhecidos.
6. Prefira XInput para pedais separados. **Forçar DirectInput** é uma alternativa de diagnóstico, com a limitação dos pedais combinados.

## Configurar o HidHide

As etapas abaixo adaptam o [guia oficial do HidHide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/) ao Boreal Wheel. A [release oficial 1.5.230](https://github.com/nefarius/HidHide/releases/tag/v1.5.230.0) informa os sistemas suportados.

1. Instale o HidHide a partir dos Assets oficiais e reinicie.
2. Abra **HidHide Configuration Client**. Em **Applications**, adicione o caminho completo de `C:\BorealWheel\boreal-wheel.exe`. Use o caminho real da sua instalação.
3. Em **Devices**, com a ponte desligada, desconecte/reconecte o volante para identificar qual entrada corresponde ao físico. Marque somente esse dispositivo.
4. Ative **Enable device hiding** e mantenha **inverse application cloak** desligado.
5. Reconecte o volante e reinicie completamente o Boreal Wheel e o jogo.

Permita o **executável nativo**: o navegador e `run.cmd` não leem o volante. Deixe o controle virtual visível e os jogos fora da lista de exceções. Se o volante sumir também do Boreal Wheel, revise o caminho permitido.

Em uma instalação com apenas esse volante, `joy.cpl` deve deixar de mostrar o físico com a ponte desligada; ativando a ponte, deve aparecer o virtual. A interface do Boreal Wheel deve continuar respondendo ao físico. A documentação oficial registra uma limitação de visibilidade em alguns dispositivos XInput; persistindo a duplicação, consulte [solução de problemas](TROUBLESHOOTING.md).

## 5. Ativar a ponte e configurar o jogo

1. Antes de abrir o jogo, confira a direção e pedais na interface.
2. Clique **Ativar ponte**. Espere **Xbox 360 virtual ativo · slot N** em Diagnóstico.
3. Confira o controle virtual em `joy.cpl`. Ele costuma ter o nome de um controle Xbox 360, não o nome Boreal Wheel.
4. Abra o jogo e selecione **controle Xbox 360 / gamepad**. Os padrões são LX para direção, RT para acelerar e LT para frear.
5. Teste em uma sessão parada. Evite manter dois remapeadores gerando saída para o mesmo volante; ajuste opções de Steam Input conforme o jogo para evitar outra tradução concorrente.

O indicador do programa representa a saída calculada; a mensagem **virtual ativo** confirma que ela está sendo entregue ao ViGEmBus. Movimento na Demo não confirma funcionamento no jogo.

## Uso diário e encerramento

Conecte o volante → `run.cmd` → **Dispositivo** → confira a leitura → **Ativar ponte** → abra o jogo. A ponte inicia desligada em cada execução. O perfil é carregado automaticamente.

Para encerrar, feche o jogo, desative a ponte e pressione `Ctrl+C` no terminal. Fechar só o navegador mantém o processo ativo. Se o dispositivo desconectar, o programa neutraliza a saída e remove o virtual; ao reconectar tenta retomar a leitura e, se a ponte continuar solicitada, recriar a saída.

## Atualização, mudança de pasta e remoção

Antes de atualizar, [exporte um perfil](PROFILES.md) e copie `profile.ini`. Encerre o processo e substitua os arquivos do pacote, preservando esse INI. Reabra e use `Ctrl+F5` no navegador.

**Migração do antigo Project B:** adicione `boreal-wheel.exe` em Applications do HidHide mesmo quando a pasta é a mesma: o nome do executável mudou. Copie o `profile.ini` antigo ou importe seu JSON/código `PB2.`. A regra do caminho antigo não permite automaticamente o novo executável.

Ao mover a instalação, inclua o novo caminho no HidHide. Para parar de ocultar o volante, desmarque **Enable device hiding** e reconecte-o. Para remover apenas o aplicativo, encerre-o, guarde seu perfil e apague a pasta. ViGEmBus e HidHide são instalações separadas; remova-os pelo Windows somente se também desejar removê-los e nenhum outro programa depender deles.

## Idiomas na versão 0.3.0

Inglês é o idioma padrão, independentemente do idioma do navegador. No seletor **Language**, escolha **Português (Brasil)** para traduzir a interface, mensagens, gráficos e atributos de acessibilidade. A preferência fica salva no navegador, separada do perfil. A troca não altera os controles, a calibração, o código digitado nem a ponte ativa.

Para mensagens do terminal/API em português, use `boreal-wheel.exe --lang pt-BR`, `run-pt-BR.cmd` ou `.\run-windows.ps1 -Language pt-BR`. No Linux: `./run.sh --lang pt-BR`. A interface traduz as mensagens suportadas do processo para o idioma escolhido nela. Identificadores técnicos e nomes de dispositivos externos mantêm sua forma original.

Chaves JSON, códigos BW2./PB2., valores numéricos e nomes de arquivos exportados são iguais nos dois idiomas. O formato de perfil continua na versão 2. Tema e idioma não são incluídos no perfil nem apagados pelo reset dos ajustes.
