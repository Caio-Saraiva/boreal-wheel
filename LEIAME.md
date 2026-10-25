# Boreal Wheel 0.3.0 — início rápido no Windows x64

[English](GETTING_STARTED.md) | **Português (Brasil)**

1. Extraia o pacote inteiro para uma pasta fixa, como `C:/BorealWheel`.
2. Confira o volante físico em `joy.cpl` e instale o driver ViGEmBus oficial.
3. Execute `run.cmd`, selecione **Dispositivo / Device** e confira a leitura.
4. No HidHide, permita o caminho completo de `boreal-wheel.exe` e oculte somente o volante físico.
5. Clique **Ativar ponte / Enable bridge**, confira **Xbox 360 virtual ativo / Virtual Xbox 360 active** e abra o jogo como controle/gamepad.

O idioma padrão é inglês. Selecione **Language → Português (Brasil)** no topo para a interface em português. `run-pt-BR.cmd` também inicia as mensagens do processo em português; `run-demo-pt-BR.cmd` é a alternativa de demonstração.

O driver é instalado separadamente; `ViGEmClient.dll` incluída é apenas a biblioteca cliente. `run-demo.cmd` permite explorar sem hardware e bloqueia a saída para jogos. Mantenha o terminal aberto enquanto joga; `Ctrl+C` encerra o processo.

## Documentação completa incluída

- [Instalação, ViGEmBus e HidHide](docs/pt-BR/INSTALLATION.md)
- [Configuração, mapeamento e calibração](docs/pt-BR/CONFIGURATION.md)
- [Exportação, importação, perfis antigos e backups](docs/pt-BR/PROFILES.md)
- [Solução de problemas](docs/pt-BR/TROUBLESHOOTING.md)
- [Visão geral e limites](README.pt-BR.md)

A migração do Project B exige adicionar o novo executável às exceções do HidHide. Preserve `profile.ini` ao atualizar. Códigos PB2. e JSON versão 2 continuam importáveis; novos perfis usam BW2./boreal-wheel. Trocar idioma não altera o formato do perfil.

A ponte inicia desligada. O perfil é salvo automaticamente na pasta de execução. Esta distribuição tem testes do núcleo/interface no Linux e build Windows x64; ainda requer validação com volante físico e jogo no Windows.
