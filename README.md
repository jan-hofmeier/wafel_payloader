# wafel_payloader

This plugin for [stroopwafel](https://github.com/shinyquagsire23/stroopwafel) allows you to load a payload.elf loader like **[PayloadFromRPX](https://github.com/wiiu-env/PayloadFromRPX)** from SD

- Patches the LoadFile IOSU functionto redirect `men.rpx` and `safe.rpx` to `sd:/wiiu/environments/aroma/root.rpx` or `sd:/wiiu/environments/tiramisu/root.rpx` or `/vol/storage_homebrew/wiiu/root.rpx` (tries each in that order). 
- undos all patches once triggered to not interfere with mocha.

## How to use

- Copy the `wafel_payloader.ipx` to `sd:/wiiu/ios_plugins`


## Building

```bash
export STROOPWAFEL_ROOT=/path/too/stroopwafel-repo
make
```


## Thanks

- [shinyquagsire23](https://github.com/shinyquagsire23)
- [GaryOrderNichts](https://github.com/GaryOderNichts) 
- [Maschell](https://github.com/Maschell)
