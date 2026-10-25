# Perfis, exportação, importação e backup

[English](../PROFILES.md) | **Português (Brasil)**

A interface inicia em inglês. Selecione **Language → Português (Brasil)** no topo para acompanhar os nomes usados neste guia.

[Voltar ao README](../../README.pt-BR.md) · [Configuração](CONFIGURATION.md) · [Formato técnico](API.md)

## O que é salvo

O programa mantém **um perfil ativo por pasta de execução**, salvo automaticamente em `profile.ini`. A interface não oferece uma biblioteca de perfis com nomes: para alternar entre jogos, exporte arquivos separados e importe o desejado.

| Incluído no perfil | Guardado separadamente |
| --- | --- |
| Fontes dos seis eixos e dos 14 botões/direcional | Tema claro/escuro no armazenamento local do navegador |
| Zona morta, limite, curva, inversão e centro por eixo | Lista de executáveis e dispositivos ocultos do HidHide |
| Fonte negativa dos quatro analógicos | Driver ViGEmBus e instalação do driver físico |
| Lado e limiar para acionar botões | Configurações internas do jogo |
| Slot XInput, DirectInput, eixos DI e frequência alvo | Modo Demo, entradas momentâneas e ativação da ponte |

Ao iniciar, o programa carrega `profile.ini`, mas **a ponte começa desligada**. Importar um perfil mantém o estado atual da ponte; se já estiver ativa, as alterações podem chegar ao jogo. Para trocar ajustes com tranquilidade, desative-a primeiro.

## Salvamento automático

1. Faça uma alteração válida na interface.
2. Aguarde a indicação de salvamento. Não há um botão geral de “Salvar”.
3. O núcleo grava `profile.ini.tmp` e substitui `profile.ini` somente quando a gravação termina.

O arquivo fica no **diretório de trabalho do processo**, não necessariamente ao lado do executável. Os scripts fornecidos escolhem esse local:

| Como iniciou | Local do INI |
| --- | --- |
| `run.cmd` do ZIP | Pasta extraída, junto ao executável |
| `run-windows.ps1` | `build-windows/Release/profile.ini` |
| `./run.sh` | Raiz do repositório |
| Executável chamado diretamente | Diretório de trabalho escolhido pelo terminal/atalho |

Se aparecer **Cannot save profile.ini**, use uma pasta gravável. Não dependa da pasta temporária do ZIP nem de um local protegido como `Program Files` sem permissão de escrita.

## Exportar como código

1. Abra **Mapeamento → Seu perfil, em um código**.
2. Aguarde qualquer ajuste pendente e clique **Gerar código**.
3. Clique **Copiar código**. Se o navegador bloquear a área de transferência, selecione todo o campo e use `Ctrl+C`.
4. Guarde o código completo, começando por **`BW2.`**, em um arquivo de texto ou nota. Dê um nome descritivo, como `jogo-direcao-suave-2026-10-25`.

O código é um JSON codificado em Base64; não é criptografia e não exige internet. Ele transporta a mesma configuração do arquivo JSON. Não remova o prefixo nem recorte partes. Se mudar ajustes depois, clique **Gerar código** novamente: o texto já presente é um retrato do momento em que foi gerado.

## Importar um código

1. Desative a ponte se estiver jogando.
2. Exporte a configuração atual para poder voltar atrás.
3. Cole o código completo no campo **Código do perfil**.
4. Clique **Aplicar perfil** e aguarde **Perfil aplicado e salvo**.
5. Confira as fontes e a calibração. Um slot XInput pode corresponder a outro dispositivo em outro PC.
6. Ative a ponte quando estiver pronto.

O campo também aceita JSON completo. Códigos antigos **`PB2.`** e JSON com `"format": "project-b"`, versão 2, continuam aceitos. Novas exportações usam `BW2.` e `"format": "boreal-wheel"`. Não há garantia de que o programa antigo aceite o formato novo.

## Exportar como arquivo JSON

Clique **Baixar arquivo** na mesma área. O navegador baixa `boreal-wheel-profile.json` para sua pasta de downloads configurada. Renomeie para algo como `meu-jogo-458-italia.json`.

Para manter vários perfis:

```text
PerfisBoreal/
  corrida-linear.json
  corrida-sensivel.json
  backup-antes-de-atualizar.json
```

O download não muda o nome nem a localização do perfil ativo. Os arquivos exportados são backups independentes; ajustes posteriores não os atualizam.

## Importar um arquivo JSON

1. Clique **Abrir arquivo**.
2. Escolha o JSON exportado. A seleção já inicia a importação; não é preciso clicar em Aplicar perfil depois.
3. Aguarde a confirmação e confira os ajustes.

São aceitos arquivos com até 40.000 bytes pela interface. A versão e todos os campos precisam ser válidos. Arquivos de diagnóstico, INI ou capturas de tela não são perfis JSON.

Importações incompletas, com campos desconhecidos, valores fora dos limites ou versão incompatível são recusadas antes de substituir o perfil salvo. Em caso de erro, a configuração anterior permanece. Use a [referência técnica](API.md) se precisar inspecionar o JSON, mas prefira editar pela interface.

## Backup direto do profile.ini

Com o processo encerrado, copie `profile.ini` para outro local. Para restaurar, encerre o programa, coloque a cópia no diretório de trabalho correto e inicie de novo. Não edite o arquivo enquanto o processo estiver ativo: a próxima alteração pela interface pode sobrescrevê-lo.

O INI é o armazenamento interno. Ele também aceita algumas chaves legadas, mas não passa pela mesma verificação de envelope da importação JSON. A carga do INI ignora linhas/chaves que não consegue interpretar; para transferir configurações entre pessoas/PCs, prefira a exportação JSON/código completa e validada.

## Atualizar ou migrar do Project B

1. No programa antigo, exporte JSON/código e faça uma cópia do INI.
2. Feche o jogo e encerre o processo antigo.
3. Extraia a nova distribuição e copie o INI para a pasta nova, ou importe o arquivo/código antigo pela interface.
4. No HidHide, adicione o **novo caminho de `boreal-wheel.exe`**. Isso é necessário mesmo quando só o nome do executável mudou.
5. Abra o programa, recarregue com `Ctrl+F5` e confira leitura, fontes, centro e curso dos pedais antes de ativar a ponte.

A preferência de tema antiga é reaproveitada se estiver disponível no mesmo endereço/origem do navegador. Trocar porta, navegador, perfil do navegador ou limpar seus dados pode mudar o tema; isso não apaga o `profile.ini` no disco.

## Compartilhar ajustes e voltar ao padrão

Compartilhe apenas o JSON de perfil ou código. Ele não contém suas credenciais GitHub, nome/email do Git ou caminhos do HidHide. O arquivo de diagnóstico pode incluir identificação do dispositivo e detalhes do ambiente; confira o conteúdo ao pedir ajuda.

Para voltar aos padrões, use **Restaurar padrão → Confirmar restauração**. Para experimentar temporariamente sem apagar nada, encerre o programa e mova o INI para um backup; sem ele, o programa inicia com os valores padrão. O [perfil de exemplo](../../examples/default-profile.json) também contém os padrões da versão atual, sem calibração específica de um volante real.

## Idiomas na versão 0.3.0

Inglês é o idioma padrão, independentemente do idioma do navegador. No seletor **Language**, escolha **Português (Brasil)** para traduzir a interface, mensagens, gráficos e atributos de acessibilidade. A preferência fica salva no navegador, separada do perfil. A troca não altera os controles, a calibração, o código digitado nem a ponte ativa.

Para mensagens do terminal/API em português, use `boreal-wheel.exe --lang pt-BR`, `run-pt-BR.cmd` ou `.\run-windows.ps1 -Language pt-BR`. No Linux: `./run.sh --lang pt-BR`. A interface traduz as mensagens suportadas do processo para o idioma escolhido nela. Identificadores técnicos e nomes de dispositivos externos mantêm sua forma original.

Chaves JSON, códigos BW2./PB2., valores numéricos e nomes de arquivos exportados são iguais nos dois idiomas. O formato de perfil continua na versão 2. Tema e idioma não são incluídos no perfil nem apagados pelo reset dos ajustes.
