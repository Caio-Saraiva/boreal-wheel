# Desenvolvimento, builds e testes

[English](../DEVELOPMENT.md) | **Português (Brasil)**

A interface inicia em inglês. Selecione **Language → Português (Brasil)** no topo para acompanhar os nomes usados neste guia.

[Voltar ao README](../../README.pt-BR.md) · [API](API.md) · [Licenças](THIRD_PARTY_NOTICES.md)

## Obter o código

```bash
git clone https://github.com/Caio-Saraiva/boreal-wheel.git
cd boreal-wheel
```

O núcleo é C11, a interface é HTML/CSS/JavaScript e a biblioteca externa ViGEmClient é C++. Não há dependência npm ou framework web. `build`, `dist`, `.deps`, `.toolchain` e o perfil pessoal ficam fora do Git.

## Linux

Ubuntu/Debian:

```bash
sudo apt install build-essential python3 cmake
./run.sh --demo
```

O script compila `build/boreal-wheel`, usa a pasta do repositório como diretório de trabalho e abre `http://127.0.0.1:4580`. `Ctrl+C` encerra.

```bash
./run.sh                       # leitura física evdev/xpad
./run.sh --demo                 # demonstração sem saída para jogos
./run.sh --port 4581            # porta alternativa
BW_NO_BROWSER=1 ./run.sh --demo # sem abrir navegador
./run.sh --build-only           # só compilar
CC=gcc ./run.sh --build-only    # escolher compilador
```

A pasta `.toolchain` é um fallback local opcional, não acompanha o clone e não é requisito. Em um clone novo, instale o compilador do sistema.

Alternativa CMake:

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
cd build-cmake
./boreal-wheel --demo
```

O CMake copia `web` para junto do executável. A leitura Linux exige acesso ao event do volante; consulte [permissões](TROUBLESHOOTING.md#linux-permissão-de-leitura). A saída virtual segue exclusiva do Windows.

## Windows x64 com Visual Studio

Instale **Visual Studio 2022 / Build Tools** com **Desktop development with C++**, Windows SDK e CMake disponível no terminal. Depois, em PowerShell na pasta do repositório:

```powershell
.\build-windows.ps1
.\run-windows.ps1
.\run-windows.ps1 -Demo
.\run-windows.ps1 -Port 4581
```

O build:

1. Configura CMake x64 e compila Release.
2. Executa o teste C de mapeamento com CTest.
3. Baixa o ZIP do commit oficial ViGEmClient fixado no script, verifica SHA256 e compila a DLL.
4. Copia `ViGEmClient.dll` e sua licença para `build-windows/Release`.

O primeiro build da dependência precisa de internet. O script não instala ViGEmBus/HidHide. Para compilar apenas o núcleo:

```powershell
.\build-windows.ps1 -SkipVigem
# Compilar a DLL separadamente depois:
.\tools\build-vigem-client.ps1
```

Sem a DLL, monitor e Demo podem funcionar, mas a ponte não alimenta o ViGEmBus. Sem o driver ViGEmBus instalado, ter a DLL também não basta.

## Compilação cruzada Linux → Windows

```bash
sudo apt install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 curl python3
./tools/build-cross.sh
python3 tools/package-windows.py
```

O script usa `x86_64-w64-mingw32-gcc` e `x86_64-w64-mingw32-g++-posix`. Se a distribuição só oferecer nomes com sufixo, configure `MINGW_CC`/`MINGW_CXX`. Uma `.toolchain/mingw` local também pode ser usada como fallback.

Saídas: `build/boreal-wheel.exe` e `build/ViGEmClient.dll`, ambos x64. O ZIP da dependência fica em `.deps/vigem`, com commit e hash fixados; alteração inesperada de hash interrompe o build. Não remova a verificação.

O empacotador Python usa esses binários de `build/`, copia a interface, os guias, as licenças e cria `run.cmd`, `run-demo.cmd`, `LEIAME.md` e `SHA256SUMS.txt`. Resultado: `dist/boreal-wheel-windows-x64.zip`. Ele empacota um build MinGW já feito; não é um instalador nem um script de build MSVC.

Os avisos de runtime MinGW são obtidos da toolchain local ou dos pacotes do sistema. Sem eles, o empacotamento falha para não gerar um pacote com notificações incompletas. Não coloque o `profile.ini` pessoal ou drivers nesse ZIP. Ao publicar, disponibilize também o hash do arquivo ZIP.

## Testes

```bash
./test.sh
# Interface: Firefox e geckodriver no PATH, sem bibliotecas Python adicionais
python3 tests/test_ui.py
```

`test.sh` compila o programa e os testes de mapeamento e executa dez testes de integração contra o servidor nativo. Eles verificam curvas, continuidade e renormalização, pedais independentes, troca de fontes, botões, pares digitais, limites, persistência, desconexão, validação de perfis e restrições HTTP.

O teste UI abre Firefox headless, interage com o processo C real e testa calibração numérica e por arraste, identificação, medição de repouso, troca dos pedais, temas, importação/exportação e rejeição atômica. Também verifica código/JSON legado após a renomeação e layout em 1440 e 390 pixels. Ele atualiza as capturas em `docs/`.

Variáveis dos testes:

| Variável | Uso |
| --- | --- |
| `BW_BINARY` | Caminho de outro binário nativo para `tests/test_api.py` |
| `GECKODRIVER` | Caminho do geckodriver para o teste UI |
| `FIREFOX_BINARY` | Caminho do Firefox para o teste UI |

Os testes isolam o perfil em diretórios temporários. Testar e compilar no Linux não comprova a instalação de drivers nem o comportamento físico no Windows. Confira manualmente o pacote no Windows antes de considerar a integração validada no hardware.

## Organização e mudanças

```text
src/bridge.h            Contratos e estruturas BW do núcleo
src/mapping.c           Fontes, curvas, pedais, botões e validação
src/main.c              Loop, snapshots, perfil e servidor HTTP local
src/i18n.c              Mensagens nativas em inglês/PT-BR
src/platform_windows.c  XInput / DirectInput / SetupAPI / ViGEm
src/platform_linux.c    sysfs / evdev / xpad
web/                    Interface, estilos, SVG e catálogos inglês/PT-BR
examples/               Perfil padrão exportável
tests/                 Testes C, API e Firefox
tools/                 Build de dependência, cross build e pacote
docs/                  Guias, referências, capturas e licença externa
```

Ao mudar o formato de perfil, atualize validação no núcleo e navegador, documentação, exemplos e testes de ida e volta. Preserve a importação legada quando possível. Não edite fontes ou licenças de terceiros para renomear o aplicativo.

## Licenças

O código do aplicativo usa MIT. A DLL vem do [ViGEmClient oficial](https://github.com/nefarius/ViGEmClient), commit `b66d02d57e32cc8595369c53418b843e958649b4`, com SHA256 do ZIP fonte `f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c`. Veja [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). ViGEmBus e HidHide não são redistribuídos pelo aplicativo.

## Localização em inglês e PT-BR

Inglês é o idioma fonte e padrão. A interface usa `web/i18n.js` e o catálogo `web/locales.js`; mensagens dinâmicas e textos em canvas usam `t(...)`. Textos estáticos e atributos de acessibilidade são traduzidos por `translateTree`. Não traduza seletores, chaves de API/perfil ou códigos digitados pelo usuário.

O núcleo usa `src/i18n.c` e `bw_text(...)`; `--lang en|pt-BR` define o idioma antes de iniciar a thread. `BW_LANG=pt-BR` define o padrão do processo e das mensagens dos scripts shell/empacotador; `--lang` prevalece no executável. Os scripts PowerShell aceitam `-Language pt-BR`. O pacote sempre inclui ambos os idiomas, com `GETTING_STARTED.md` em inglês, `LEIAME.md` em português e launchers localizados.

O teste Firefox executa os fluxos completos em inglês e PT-BR, verifica idioma inicial, troca sem perda de configuração/texto digitado, persistência e acessibilidade. Capturas em inglês ficam em `docs/` e em português em `docs/pt-BR/images/`. O núcleo/API também testa seleção e validação do idioma.
