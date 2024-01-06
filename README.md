# wafel_payloader

This plugin for [stroopwafel](https://github.com/shinyquagsire23/stroopwafel) allows you to load a payload.elf loader like **[PayloadFromRPX](https://github.com/wiiu-env/PayloadFromRPX)** from SD

- Patches the LoadFile IOSU functionto redirect `men.rpx` to `sd:/wiiu/root.rpx`
- undos all patches once triggered to not interfere with mocha.

## How to use

- Copy the `wafel_payloader.ipx` to `/wiiu/ios_plugins`
- Copy the PayloadFromRPX to `sd:/wiiu/root.rpx`


## Building

```bash
export STROOPWAFEL_ROOT=/path/too/stroopwafel-repo
make
```


## Thanks

- [shinyquagsire23](https://github.com/shinyquagsire23)
- [GaryOrderNichts](https://github.com/GaryOderNichts) 
- [Maschell](https://github.com/Maschell)
