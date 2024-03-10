# sxsfounder

Creates foundation image from SxS components.

Requires manifests to be uncompressed, at root of SxS source folder. Use [SxSExpand](https://github.com/hfiref0x/SXSEXP) to convert existing SxS sources to this format if needed.

> DISCLAIMER: This code is very rough and thoroughly untested. If it deletes your Minecraft hardcore world or makes you grow a third head, that is strictly a "you" problem.

## Usage
`sxsfounder <sxs source folder> <deployment manifest> <offline image path>`

Requires the following servicing stack DLLs, which can be sourced from host WinSxS:
 - `wcp.dll`
 - `smiengine.dll`
 - `smipi.dll`

An example deployment manifest can be found [here](./SxSFounder-Offline-Servicing-Deployment.manifest).

## Credits
- [asdcorp](https://github.com/asdcorp) for [haveSxS](https://github.com/asdcorp/haveSxS) and their invaluable education on servicing
- [seven-mile](https://github.com/seven-mile) for [CallCbsCore](https://github.com/seven-mile/CallCbsCore) and [UF-Case](https://github.com/seven-mile/UFCase)
- [himselfv](https://github.com/himselfv) for [manifestenum](https://github.com/himselfv/manifestenum)
