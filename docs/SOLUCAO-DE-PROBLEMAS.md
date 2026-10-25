# Solução de problemas

[Voltar ao README](../README.md) · [Instalação](INSTALACAO.md) · [Perfis](PERFIS.md)

## Qual etapa falhou?

Valide nesta ordem: **driver físico → entrada no Boreal Wheel → saída virtual → HidHide → jogo**. Comece com o jogo fechado. Desative temporariamente a ocultação do HidHide quando precisar testar o físico no Windows, e reative-a ao terminar.

| Sintoma | Confira primeiro |
| --- | --- |
| Nada aparece nem no Gerenciador de Dispositivos | Cabo, porta USB, conexão dos pedais e outro PC; orientação do fabricante |
| USB detectado, mas sem entrada | Driver físico, slot XInput, exceção do HidHide |
| Interface responde, mas o jogo não | Dispositivo versus Demo, ponte ativa, ViGEmBus, seleção do controle no jogo |
| Menus pulam ou ações saem duas vezes | Físico e virtual visíveis ao mesmo tempo; HidHide e outros remapeadores |
| Pedal errado ou acionado em repouso | Fontes RT/LT, inversão, eixo combinado DI e zona morta |
| Ajustes somem ao reiniciar | Local do `profile.ini`, erro de gravação ou duas instâncias |

## Página não abre / núcleo offline

- Execute `run.cmd` ou o script de desenvolvimento e mantenha o terminal aberto. Abrir somente o HTML não inicia o núcleo.
- Use `http://127.0.0.1:4580`, com **http**, sem trocar por `localhost`. O servidor valida esse endereço.
- Se o navegador abriu antes do núcleo, atualize. Depois de uma atualização do pacote, use `Ctrl+F5`.
- Se a porta estiver ocupada, encerre a instância anterior. Alternativa em terminal aberto na pasta extraída:

```powershell
.\boreal-wheel.exe --port 4581
```

Acesse `http://127.0.0.1:4581`. No build Windows: `.\run-windows.ps1 -Port 4581`. Linux: `./run.sh --port 4581`.

**UI assets missing** significa que a pasta `web` não foi encontrada no diretório de trabalho. Use o script fornecido ou abra o terminal na pasta que contém `web`. **Invalid port**: use um inteiro entre 1024 e 65535.

## Volante invisível ou USB sem leitura

1. Desligue a ponte e a ocultação do HidHide; reconecte o volante.
2. Confira `devmgmt.msc` e `joy.cpl`. Corrija primeiro o driver Xbox 360 seguindo o [fabricante](https://support.thrustmaster.com/en/kb/101-en/).
3. Em **Diagnóstico → Entrada XInput física**, tente o slot correto entre 0 e 3. Automático exige subtipo volante, e alguns drivers apresentam o equipamento como gamepad.
4. Teste **Forçar DirectInput** apenas como alternativa. Os IDs/modelos reconhecidos continuam limitados ao alvo do projeto.
5. Se voltar a falhar ao reativar o HidHide, confira o caminho completo permitido para `boreal-wheel.exe` e reinicie o processo. Não basta permitir `run.cmd` ou o navegador.

Mudar os slots de outros controles, reconectar USB e criar/remover controles virtuais pode alterar a enumeração. Confira novamente o slot físico. O programa exclui seu próprio slot virtual para evitar realimentação.

## Saída virtual não ativa

| Mensagem | Ação |
| --- | --- |
| Demonstração · saída para jogos bloqueada | Selecione Dispositivo. Demo não pode alimentar jogos. |
| Aguardando entrada física · saída neutralizada | Resolva a leitura física; a ponte não alimenta o jogo sem ela. |
| ViGEmClient.dll ausente ao lado do executável | Extraia a distribuição completa. Confira se o arquivo está ao lado do EXE e se consegue ser carregado. |
| ViGEmClient.dll incompatível · confira build x64 | Use a DLL x64 do mesmo pacote, sem misturar builds/arquiteturas. |
| ViGEmBus não disponível · instale o driver oficial | Instale/repare o driver oficial, conclua as reinicializações e reabra o aplicativo. |
| Falha ao criar Xbox 360 virtual no ViGEmBus | Confira driver e outros remapeadores. Libere controles virtuais desnecessários e reinicie o processo. |
| Não foi possível identificar o slot virtual com segurança | Confira os quatro slots XInput disponíveis e conflitos; a saída é removida quando o slot não pode ser confirmado. |
| Falha ao enviar relatório ao ViGEmBus | Confira o driver, desative/ative a ponte e reinicie o processo se necessário. |
| Prévia Linux · saída XInput disponível apenas no Windows | Comportamento esperado no Linux; use a versão Windows para essa saída. |

O programa tenta novamente após falhas de conexão ViGEmBus. Criar o dispositivo virtual pode levar um instante; confira **Xbox 360 virtual ativo** antes de abrir o jogo.

## Comandos duplicados ou jogo prefere o volante original

O HidHide deve ocultar **somente o físico**, deixando o Boreal Wheel ler esse físico e o jogo ver o virtual. Confira Applications, caminho do novo executável, Devices e a opção de ocultação conforme o [guia de instalação](INSTALACAO.md#configurar-o-hidhide).

Feche outros remapeadores e reabra o jogo depois de criar o virtual. Se houver Steam Input ou outro sistema de tradução, experimente a opção apropriada por jogo para evitar duas traduções simultâneas. Alguns jogos guardam o dispositivo escolhido; selecione novamente o controle Xbox 360/gamepad.

Se você ocultou o virtual por engano, desligue a ponte e use a conexão física do volante para identificar a entrada correta. Para desfazer a ocultação, desmarque **Enable device hiding**, reconecte o volante e reabra os aplicativos. O [guia oficial](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/) registra que alguns clientes XInput podem continuar mostrando dispositivos marcados.

## Pedais ou direção incorretos

- **Pedais trocados:** use **Trocar acelerador e freio**, depois confira a calibração de cada saída.
- **Pedal em 100% quando solto:** confira a fonte e **Inverter sentido**. Não resolva isso com zona morta gigante.
- **Acelerador e freio se anulam:** veja se o backend é DirectInput e se a interface informa pedais combinados. Esse caminho não recupera os dois pressionados simultaneamente; prefira XInput, conforme a [documentação Microsoft](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput).
- **Direção chega ao máximo cedo:** aumente **100% de saída ao mover**; confira também a sensibilidade no jogo.
- **Direção exige movimento demais:** reduza esse limite aos poucos.
- **Movimento sozinho:** capture o centro com o volante solto e ajuste a zona morta para cobrir o ruído.
- **Identificar escolhe outro comando:** estabilize os controles, mova um de cada vez ou escolha a fonte manualmente.
- **Capturar máximo recusado:** mantenha o eixo/pedal além do mínimo de 5% e com pelo menos 2 pontos percentuais acima da zona morta.

Não há force feedback nem saída nativa de volante nesta versão. O jogo pode aplicar filtros de gamepad por cima dos ajustes do Boreal Wheel.

## Perfil não importa ou não fica salvo

Códigos aceitos: `BW2.` e legado `PB2.`. Arquivo aceito: JSON de perfil versão 2 completo. Não importe o JSON de diagnóstico como perfil. Confira a mensagem junto ao campo e leia o [guia de perfis](PERFIS.md).

Se os ajustes persistirem só até fechar o programa, confira a permissão de escrita e o diretório de trabalho do processo. Não mantenha duas instâncias salvando o mesmo INI. Faça uma cópia antes de editar/mover o arquivo.

## Linux: permissão de leitura

O backend procura IDs conhecidos em `/sys/class/input` e abre `/dev/input/eventN`. Se houver USB detectado, mas acesso negado ao event, teste uma permissão temporária específica depois de identificar o nó do volante:

```bash
sudo setfacl -m "u:$(id -un):r" /dev/input/eventN
```

Substitua `eventN` pelo nó real; ele pode mudar ao reconectar. Isso não cria uma regra permanente. Confira as permissões da sua distribuição, udev e o módulo `xpad` para uma solução persistente. Não é necessário executar toda a interface como root. Linux segue sem saída XInput neste projeto.

## Pedir ajuda com evidências

Em **Diagnóstico**, use **Exportar diagnóstico** e confira o JSON antes de compartilhar. Informe versão do programa, Windows/Linux, modelo exato, backend, etapa que falhou, mensagem do terminal e se a ocultação estava ativa. Diga se a leitura funciona em `joy.cpl` com HidHide desligado. Compartilhe o perfil separadamente quando o problema envolver mapeamento/calibração.
