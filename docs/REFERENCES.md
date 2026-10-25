# Fontes técnicas

Referências da implementação original; guias de ViGEmBus, HidHide e driver físico conferidos novamente em 25/10/2026. Apenas documentação do fabricante, Microsoft, mantenedor e código upstream.

- [Thrustmaster — Ferrari 458 Italia de Xbox 360](https://support.thrustmaster.com/en/product/ferrari458italia-en/): identifica o modelo e o driver Microsoft Xbox 360.
- [Thrustmaster KB 101 — dispositivo Xbox 360 não detectado](https://support.thrustmaster.com/en/kb/101-en/): vínculo com o driver padrão e diagnóstico no Windows.
- [Microsoft — XInput e DirectInput](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput): gatilhos combinados em DirectInput e APIs de leitura.
- [Microsoft — versões de XInput](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-versions): XInput do sistema operacional.
- [Linux upstream — xpad.c](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c): IDs `24C6:5B00` e `24C6:5B03` do Ferrari 458 Racing Wheel; convenções dos eixos Xbox 360.
- [Nefarius — ViGEmClient](https://github.com/nefarius/ViGEmClient): ABI nativa, estrutura XUSB e integração com o driver virtual. Commit fixado: `b66d02d57e32cc8595369c53418b843e958649b4`.
- [Nefarius — ViGEmBus e releases oficiais](https://github.com/nefarius/ViGEmBus/releases): dependência do dispositivo Xbox 360 virtual.
- [Nefarius — fim do suporte ViGEm](https://docs.nefarius.at/projects/ViGEm/End-of-Life/): descontinuação e recomendações para instalações legadas.
- [Nefarius — HidHide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/): evitar exposição simultânea do controle físico e virtual a jogos.

Não foram usados drivers de sites de terceiros, projetos para o 458 Spider nem o driver do TX como substitutos para o Italia Xbox 360.

- [HidHide 1.5.230 — release oficial](https://github.com/nefarius/HidHide/releases/tag/v1.5.230.0): instalador e suporte Windows 10/11 Intel/AMD x64.
- [ViGEmBus 1.22.0 — release final](https://github.com/nefarius/ViGEmBus/releases/tag/v1.22.0): remoção do atualizador legado.
