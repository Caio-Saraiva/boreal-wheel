# Boreal Wheel

**Seu volante de volta à pista.** Ponte de entrada escrita em C11 para usar o **Thrustmaster Ferrari 458 Italia de Xbox 360** como um **controle Xbox 360 virtual / XInput** em jogos no Windows. Inclui mapeamento de comandos, calibração visual, perfis portáteis e interface local com temas claro e escuro.

Criado por **CaioSaraiva**. O nome Boreal Wheel combina a referência ao volante com o visual de vidro e luz da interface.

![Boreal Wheel em modo demonstração](docs/preview-desktop.png)

## Baixar e começar no Windows

1. Baixe `boreal-wheel-windows-x64.zip` em [Releases](https://github.com/Caio-Saraiva/boreal-wheel/releases/latest) e **extraia tudo** para uma pasta fixa, por exemplo `C:\BorealWheel`.
2. Conecte o volante e confira a leitura no Windows com `Win+R` → `joy.cpl`.
3. Instale o [ViGEmBus oficial](https://github.com/nefarius/ViGEmBus/releases/tag/v1.22.0) e reinicie se solicitado. A DLL do pacote não substitui esse driver.
4. Execute `run.cmd` e abra **http://127.0.0.1:4580**. Selecione **Dispositivo**, confira os movimentos e ajuste os comandos.
5. Configure o [HidHide](docs/INSTALACAO.md#configurar-o-hidhide) quando o jogo também detectar o volante físico: permita `boreal-wheel.exe` e oculte somente o dispositivo físico.
6. Clique **Ativar ponte**. Com a indicação **Xbox 360 virtual ativo**, abra o jogo como **controle/gamepad**: direção em LX, acelerador em RT e freio em LT.

Mantenha o terminal aberto enquanto joga. Fechar a aba não encerra a ponte. Para parar, clique **Desativar ponte** ou encerre o processo com `Ctrl+C`. `run-demo.cmd` permite explorar sem hardware e bloqueia a saída para jogos.

## Documentação

| Quero… | Guia |
| --- | --- |
| Instalar e fazer funcionar com ViGEmBus e HidHide | [Instalação completa no Windows](docs/INSTALACAO.md) |
| Entender todas as telas e configurar cada comando | [Configuração e calibração](docs/CONFIGURACAO.md) |
| Exportar, importar, compartilhar, restaurar ou migrar ajustes | [Perfis e backups](docs/PERFIS.md) |
| Resolver volante invisível, comandos duplicados, pedais ou falhas | [Solução de problemas](docs/SOLUCAO-DE-PROBLEMAS.md) |
| Compilar, testar, empacotar ou executar no Linux | [Desenvolvimento](docs/DESENVOLVIMENTO.md) |
| Automatizar ou entender o formato de configuração | [Referência da API e do perfil](docs/API.md) |
| Conferir as fontes e licenças | [Referências](docs/REFERENCES.md) · [Componentes externos](THIRD_PARTY_NOTICES.md) |

## Recursos e compatibilidade

- Seis saídas analógicas: LX, LY, RX, RY, RT e LT; 14 botões/direcional. Eixos e botões físicos podem alimentar qualquer saída disponível.
- Identificação de entradas por movimento, troca de pedais e dois botões para os dois lados de um analógico.
- Zona morta, fim do curso/sensibilidade, curva, inversão e centro por eixo; gráficos e ajustes por mouse, toque, números e sliders.
- Salvamento automático em `profile.ini`, exportação em JSON ou código `BW2.`, importação dos antigos `PB2.` e diagnóstico exportável.
- Interface HTML/CSS/JavaScript local, sem npm, fontes remotas ou conta. O núcleo continua funcionando com a aba fechada.

| Ambiente | Suporte |
| --- | --- |
| Windows x64 | Entrada XInput 1.4, fallback DirectInput 8, diagnóstico USB e saída ViGEm Xbox 360. Windows 11 é o alvo principal. |
| Linux | Entrada evdev/xpad dos IDs `24C6:5B00` e `24C6:5B03`, interface e Demo; **sem saída virtual para jogos**. |
| Outros volantes | Sem garantia de detecção. O alvo é o **458 Italia Xbox 360**, não o 458 Spider Xbox One ou o TX. |

O jogo recebe um **gamepad**, portanto o aplicativo não acrescenta suporte nativo de volante, force feedback, botão Guide ou sensores ausentes no hardware. DirectInput combina os pedais e não preserva aceleração e frenagem simultâneas; prefira XInput. Há até quatro slots XInput no sistema.

O [ViGEm foi descontinuado pelo mantenedor](https://docs.nefarius.at/projects/ViGEm/End-of-Life/). Boreal Wheel não instala drivers automaticamente. `ViGEmClient.dll` x64 acompanha o pacote; **ViGEmBus e HidHide são instalados separadamente**. O executável desta versão não possui assinatura digital de publicação.

## Executar a partir do código

```bash
git clone https://github.com/Caio-Saraiva/boreal-wheel.git
cd boreal-wheel
# Linux: compilador C e bibliotecas de desenvolvimento
./run.sh --demo
```

No Windows, com Visual Studio/Build Tools, Windows SDK e CMake:

```powershell
.\build-windows.ps1
.\run-windows.ps1
```

Veja os requisitos e alternativas no [guia de desenvolvimento](docs/DESENVOLVIMENTO.md).

## Estado da versão 0.2.0

A base anterior teve leitura do volante físico e uso em jogo relatados no Windows com HidHide. A renomeação para Boreal Wheel é verificada por testes do mapeamento C, dez testes de integração da API, testes da interface em Firefox e compilação cruzada Windows x64. As capturas são do modo Demo.

**Ainda requer validação no hardware:** esta distribuição renomeada, a calibração e o fluxo de perfis no Windows com volante e jogo reais. Testes no Linux e compilação não substituem essa conferência.

Código sob [licença MIT](LICENSE). Este projeto é independente de Thrustmaster, Ferrari, Microsoft e Nefarius.
