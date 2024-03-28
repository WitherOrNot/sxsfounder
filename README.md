# sxsfounder

Creates foundation image from SxS components. Requires `cbss.exe` to be serviced. If you don't know what that means, don't bother.

> DISCLAIMER: This code is very rough and thoroughly untested. If it deletes your Minecraft hardcore world or makes you grow a third head, that is strictly a "you" problem.

## Usage
`sxsfounder <offline image path> <architecture>`

Where `<architecture>` is one of:
 - `x86`
 - `amd64`
 - `arm`
 - `arm64`

Requires `wcp.dll` which can be sourced from host or imaged WinSxS.

## Credits
- [seven-mile](https://github.com/seven-mile) for [CallCbsCore](https://github.com/seven-mile/CallCbsCore) and [UF-Case](https://github.com/seven-mile/UFCase)
