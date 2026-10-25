# Referência da API e do formato de perfil

[Voltar ao README](../README.md) · [Uso dos perfis](PERFIS.md) · [Desenvolvimento](DESENVOLVIMENTO.md)

Referência da implementação 0.2.0. Para uso comum, configure pela interface. A API é local, serve o navegador e não oferece descoberta de dispositivos arbitrários nem acesso remoto autenticado.

## Endereço e requisições

Padrão: `http://127.0.0.1:4580`. O processo escuta apenas loopback. `--port N` aceita 1024–65535; use o mesmo número no Host e no Origin.

- GET retorna estado, perfil ou recursos estáticos permitidos.
- POST exige `Origin: http://127.0.0.1:4580` e `Content-Type: application/x-www-form-urlencoded` exatamente.
- O corpo contém pares de chaves e valores numéricos, ou `action=nome`. Não envie JSON no POST.
- Host deve ser `127.0.0.1:porta`. `localhost`, origens externas, chaves duplicadas, chunked e comprimentos ambíguos são recusados.
- A interface limita o texto/arquivo de importação a 40.000 caracteres/bytes respectivamente; o corpo HTTP nativo tem limite declarado de 16.384 bytes e precisa caber no buffer da requisição.

Exemplo: leia o perfil sem alterar nada:

```bash
curl http://127.0.0.1:4580/api/profile
```

Exemplo de ajuste parcial (altera o perfil ativo e salva):

```bash
curl -X POST http://127.0.0.1:4580/api/config \
  -H 'Origin: http://127.0.0.1:4580' \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data 'axis_deadzone_0=0.03&axis_limit_0=0.80'
```

Em PowerShell use `curl.exe` e a sintaxe de continuação apropriada, ou envie tudo em uma linha.

## Rotas

| Método e caminho | Resultado |
| --- | --- |
| `GET /api/state` | Snapshot de entrada, saída, configuração e diagnóstico |
| `GET /api/profile` | Envelope JSON com os 87 campos atuais de configuração |
| `POST /api/config` | Ajuste parcial com validação e gravação antes de substituir o perfil |
| `POST /api/profile` | Substituição completa; exige exatamente os 87 campos permitidos |
| `POST /api/action` | Ação de modo, ponte, reset ou captura |
| `POST /api/demo` | Estado sintético manual: steering/ly/rx/ry de −1 a 1; throttle/brake de 0 a 1; buttons de 0 a 65535 inteiro |
| `GET /`, `/app.js`, `/style.css`, `/favicon.svg` | Interface e recursos locais |

`/api/demo` define um snapshot completo de entrada manual: campos omitidos voltam a zero. Ele só alimenta a leitura quando Demo está em modo manual; não cria saída para o jogo.

POST válido retorna `{"ok":true}`. Erros retornam texto: 400 para ajuste/ação inválidos ou falha de gravação, 403 para Host/origem/tipo inadequados, 404 para caminhos sem rota. Não há endpoint para ler arquivos arbitrários do disco ou para servir os guias Markdown.

### Ações

Envie `action=nome` para `/api/action`:

| Nome | Efeito |
| --- | --- |
| `physical` | Seleciona hardware e desliga a solicitação da ponte |
| `demo` | Seleciona Demo automática e desliga a solicitação da ponte |
| `manual` | Seleciona interação manual da Demo e zera o estado sintético |
| `auto` | Retoma a animação automática da Demo |
| `start` / `stop` | Solicita ativação/desativação da ponte; start é recusado na Demo |
| `reset` | Salva os padrões e substitui o perfil; não muda o estado da ponte |
| `center` ou `center_0` até `center_3` | Captura o centro do eixo selecionado; exige entrada disponível e centro dentro de ±0,5 |
| `limit_0` até `limit_5` | Captura a magnitude atual como limite; exige entrada, mínimo 0,05 e separação de 0,02 da zona morta |

“Medir repouso” e “Identificar” são operações da interface que observam snapshots e enviam ajustes; não são endpoints dedicados.

### Snapshot de estado

`/api/state` contém:

- `demo`, `demo_auto`, `connected`, `physical_connected`, `usb_present`, `combined_pedals`: indicadores 0/1. Demo pode ter connected=1 sem hardware; physical_connected diferencia os casos.
- `output_requested` e `output_active`: solicitação e estado efetivo da saída, que podem diferir sem entrada/driver.
- `platform`, `name`, `backend`, `detail`, `output_status`: identificação e mensagens.
- `map_us`, `observed_hz`, `packets`: duração da transformação, frequência observada do loop e contador de iterações; packets não garante pacotes USB novos.
- `raw`: direção, pedais, LY/RX/RY, máscara de botões e array de seis eixos do backend.
- `input`: seis entradas selecionadas após centro/inversão e antes da curva/zona morta.
- `report`: LX/LY/RX/RY inteiros de −32768 a 32767, LT/RT de 0 a 255 e máscara de botões.
- `config`: configuração atual, incluindo aliases legados para compatibilidade. Não copie todos esses aliases para `/api/profile`: use `GET /api/profile` para o conjunto exportável.

## Envelope exportado

O [exemplo completo](../examples/default-profile.json) contém todos os padrões. A forma geral é:

```json
{
  "format": "boreal-wheel",
  "version": 2,
  "settings": {
    "poll_hz": 250,
    "slot": -1
  }
}
```

**O trecho acima é ilustrativo e incompleto; não é importável.** `settings` deve conter exatamente os 87 campos do perfil completo: cinco globais, 40 de eixos e 42 de botões. Valores são números JSON finitos, com inteiro onde indicado. Não acrescente comentários ao JSON.

Código portátil: `BW2.` + Base64 do texto JSON. A interface também importa prefixo legado `PB2.` e formato `project-b` com versão 2. Ela valida o envelope e envia apenas `settings` como pares de formulário ao núcleo. O núcleo valida campos, faixas e consistência antes de gravar/substituir; uma importação inválida preserva o perfil anterior.

### Campos globais

| Campo | Padrão | Valores |
| --- | --- | --- |
| `poll_hz` | 250 | 125, 250, 500 ou 1000 |
| `slot` | −1 | −1 automático ou 0–3 físico XInput |
| `directinput` | 0 | 0 automático/XInput; 1 força DI |
| `steering_axis` | 0 | 0–5: X, Y, Z, RX, RY, RZ em DI |
| `pedal_axis` | 2 | 0–5: eixo de pedais combinados em DI |

### Campos de eixo

Índice `i`: **0=LX/direção, 1=LY, 2=RX, 3=RY, 4=RT/acelerador, 5=LT/freio**.

| Campo para cada i | Padrão | Limites |
| --- | --- | --- |
| `axis_source_i` | i | Fonte −1 ou válida na tabela abaixo |
| `axis_invert_i` | 0 | 0/1 |
| `axis_deadzone_i` | 0,02 nos analógicos; 0,01 nos pedais | 0–0,6 |
| `axis_limit_i` | 1 | 0,05–1 |
| `axis_gamma_i` | 1 | 0,2–3 |
| `axis_center_i` | 0 | −0,5–0,5 |
| `axis_negative_i` | −1 | Somente i=0–3; fonte −1 ou válida |

`axis_limit_i − axis_deadzone_i` precisa ser pelo menos 0,02 (com tolerância numérica interna). Não envie `axis_negative_4` ou `_5`. Uma saída com source −1 permanece neutra inclusive se invertida.

### Fontes

| ID | Entrada |
| --- | --- |
| −1 | Desativado |
| 0, 1, 2, 3 | Direção/LX, LY, RX, RY normalizados |
| 4, 5 | Acelerador/RT e freio/LT normalizados |
| 6–15 | D-pad cima, baixo, esquerda, direita, Start, Back, LS, RS, LB, RB |
| 16, 17 | Reservados; inválidos como fonte |
| 18–21 | A, B, X, Y |
| 22–27 | Seis eixos brutos do backend, na ordem do array raw.axes |

As fontes brutas dependem do backend; “bruto” aqui é a leitura normalizada exposta pelo driver, não dados USB em bytes. Os eixos/controles ausentes no volante não passam a existir por serem selecionáveis.

### Campos de botão

O índice `b` corresponde ao bit XInput. São válidos 0–9 e 12–15. Bits 10 e 11 são reservados e não entram no perfil.

| Bit b | Saída |
| --- | --- |
| 0, 1, 2, 3 | D-pad cima, baixo, esquerda, direita |
| 4, 5 | Start, Back |
| 6, 7 | LS, RS |
| 8, 9 | LB, RB |
| 12, 13, 14, 15 | A, B, X, Y |

Para cada bit válido:

| Campo | Padrão | Valores |
| --- | --- | --- |
| `button_source_b` | 6+b | Fonte −1 ou válida |
| `button_direction_b` | 1 | −1 ou 1 |
| `button_threshold_b` | 0,5 | 0,01–1 |

O botão ativa quando `fonte × direction >= threshold`. source −1 fica desligado. O limiar é aplicado à fonte original e não depende da calibração de outro eixo de saída.

## Armazenamento INI e compatibilidade

`profile.ini` usa `chave=valor`, sem envelope. Escrita temporária e substituição reduzem o risco de um perfil parcial. O caminho é relativo ao diretório de trabalho; veja [locais dos perfis](PERFIS.md).

O leitor aceita aliases antigos como `deadzone`, `gamma`, `center`, `pedal_deadzone`, `invert_steering`, `invert_throttle`, `invert_brake` e `button_b`. Esses aliases existem para migração de INI e ajustes parciais; o perfil completo exportável usa apenas os campos atuais. A leitura do INI ignora linhas que não entende e só aplica o candidato se os limites entre eixos forem consistentes; ela não é o mesmo contrato estrito da importação pela interface.

Tema, regras do HidHide e solicitação de ponte não pertencem ao formato de perfil. Não adicione credenciais ou configurações pessoais ao repositório.
