# Third-party notices

**English** | [Português (Brasil)](docs/pt-BR/THIRD_PARTY_NOTICES.md)

The interface, core and wheel SVG were created for this project. This is not official Thrustmaster, Ferrari or Microsoft software. Product names identify the hardware and compatibility target.

The Windows package includes **ViGEmClient.dll**, built from [nefarius/ViGEmClient](https://github.com/nefarius/ViGEmClient), commit `b66d02d57e32cc8595369c53418b843e958649b4`, under the MIT license, copyright 2017–2023 Nefarius Software Solutions e.U. and Contributors. The complete license is in [docs/LICENSE-ViGEmClient.txt](docs/LICENSE-ViGEmClient.txt) and is included alongside the DLL in the Windows package.

SHA256 of the official source ZIP used for the build: `f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c`.

The packaged DLL is built with MinGW-w64 and a static runtime. The MinGW-w64 runtime contains public-domain and permissively licensed components; relevant notices are included in `LICENSES-MinGW/`. GCC/libgcc/libstdc++ use the GNU GPL with the [GCC Runtime Library Exception](https://www.gnu.org/licenses/gcc-exception-3.1.html), as applicable to the linked runtime. Full license texts are included in the package. Original third-party license texts are retained in their original language.

The **ViGEmBus** and **HidHide** drivers are neither redistributed nor installed by this project. They have their own terms and support lifecycles.
