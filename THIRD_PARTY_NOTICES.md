# Componentes externos

A interface, o núcleo e o desenho SVG do volante foram criados para este projeto. Não são software oficial Thrustmaster, Ferrari ou Microsoft. Nomes de produtos identificam o hardware e o alvo de compatibilidade.

O pacote Windows inclui **ViGEmClient.dll**, compilada a partir de [nefarius/ViGEmClient](https://github.com/nefarius/ViGEmClient), commit `b66d02d57e32cc8595369c53418b843e958649b4`, sob licença MIT, copyright 2017–2023 Nefarius Software Solutions e.U. and Contributors. A licença integral está em [docs/LICENSE-ViGEmClient.txt](docs/LICENSE-ViGEmClient.txt) e acompanha a DLL no pacote Windows.

SHA256 do arquivo-fonte oficial ZIP usado no build: `f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c`.

A DLL do pacote foi compilada por MinGW-w64 com runtime estático. O runtime MinGW-w64 contém componentes de domínio público e licenças permissivas; as notificações pertinentes acompanham o pacote em `LICENSES-MinGW/`. GCC/libgcc/libstdc++ estão sujeitos à GNU GPL com [GCC Runtime Library Exception](https://www.gnu.org/licenses/gcc-exception-3.1.html), aplicável ao runtime vinculado. As licenças completas estão no pacote.

O driver **ViGEmBus** e o **HidHide** não são redistribuídos nem instalados pelo projeto. Eles têm seus próprios termos e ciclos de suporte.
