# M5AtomS3-UserDemo

M5AtomS3 user demo for hardware evaluation.

## Build

### Fetch Dependencies

```bash
python ./fetch_repos.py
```

## Desktop Build

#### Tool Chains

```bash
sudo apt install build-essential cmake libsdl2-dev
```

#### Build

```bash
mkdir build && cd build
```
```bash
cmake .. && make -j8
```
#### Run

```bash
cd desktop && ./app_desktop_build
```

## AtomS3 Build

#### Tool Chains

[ESP-IDF v5.1.4](https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32s3/index.html)

#### Build

```bash
cd platforms/atoms3
```

```bash
idf.py build
```

#### Flash

```bash
idf.py -p <YourPort> flash -b 1500000
```

##### Flash AssetPool

```bash
parttool.py --port <YourPort> write_partition --partition-name=assetpool --input "path/to/AssetPool.bin"
```

If you run desktop build before, you can found `AssetPool.bin` at 

`../../build/desktop/AssetPool.bin`. Or download one from the [release](https://github.com/m5stack/VAMeter-Firmware/releases/latest) page.

## AtomS3R Build

#### Tool Chains

[ESP-IDF v5.1.4](https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32s3/index.html)

#### Build

```bash
cd platforms/atoms3r
```

```bash
idf.py build
```

#### Flash

```bash
idf.py -p <YourPort> flash -b 1500000
```

##### Flash AssetPool

```bash
parttool.py --port <YourPort> write_partition --partition-name=assetpool --input "path/to/AssetPool.bin"
```

If you run desktop build before, you can found `AssetPool.bin` at 

`../../build/desktop/AssetPool.bin`. Or download one from the [firmware](./firmware/) page.

