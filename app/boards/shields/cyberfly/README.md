# CyberFly - ZMK Bluetooth Firmware

基于 Nice!Nano (nRF52840) 的 CyberFly 蓝牙键盘固件，使用 ZMK 固件框架。

## 硬件需求

- **Nice!Nano v2** (nRF52840) 蓝牙开发板 (带 USB Type-C)
- CyberFly 键盘矩阵 (6行 x 12列，共72键)

## 引脚接线

Nice!Nano 使用 Pro Micro 兼容引脚排列，共 18 个 GPIO，刚好满足 6 行 + 12 列：

| 功能 | Nice!Nano 引脚 | Pro Micro 编号 |
|------|----------------|----------------|
| ROW0 | P0.08 | D0 |
| ROW1 | P0.06 | D1 |
| ROW2 | P0.17 | D2 |
| ROW3 | P0.20 | D3 |
| ROW4 | P0.22 | D4 |
| ROW5 | P0.24 | D5 |
| COL0 | P1.00 | D6 |
| COL1 | P0.11 | D7 |
| COL2 | P1.04 | D8 |
| COL3 | P1.06 | D9 |
| COL4 | P0.09 | D10 |
| COL5 | P0.10 | D16 |
| COL6 | P1.11 | D14 |
| COL7 | P1.13 | D15 |
| COL8 | P1.15 | D18 |
| COL9 | P0.02 | D19 |
| COL10 | P0.29 | D20 |
| COL11 | P0.31 | D21 |

## 键盘布局

默认键位与 QMK 版本一致，6x12 正交布局：

```
Row 0: ESC    F1     ___    F2     F3     F4     F5     F6     BSPC   -      [      ENTER
Row 1: `      1      2      3      4      5      6      7      8      9      0      =
Row 2: E      Q      W      TAB    R      T      Y      U      I      O      P      ]
Row 3: Fn     A      S      D      F      G      H      J      K      L      ;      '
Row 4: LSHFT  Z      X      C      V      B      N      M      ,      .      /      UP
Row 5: LCTRL  LGUI   LALT   \      SPC    SPC    SPC    RALT   LEFT   DOWN   RIGHT  RSHFT
```

**Fn 层**: 按住 Fn 键激活，提供 F1-F12、DEL、PgUp/PgDn、Home/End 等功能键。

**BT 层 (Layer 2)**: 蓝牙管理，支持切换 5 个蓝牙设备、清除配对、切换 USB/BLE 输出。

## 构建固件

### 方法一：GitHub Actions 自动构建（推荐）

1. Fork 本仓库到你的 GitHub 账号
2. Push 代码后会自动触发 `Build CyberFly` workflow
3. 也可以到 Actions 页面手动触发 `workflow_dispatch`
4. 构建完成后，在 Actions 的 Artifacts 中下载 `cyberfly-nice_nano_nrf52840.uf2`

### 方法二：本地构建

```bash
# 1. 安装依赖 (需要 Python 3.8+, CMake 3.20+, dtc)
pip3 install west

# 2. 初始化 workspace
cd zmk_cyberfly
west init -l app
west update

# 3. 构建固件
west build -s app -b nice_nano_nrf52840_zmk -- -DSHIELD=cyberfly

# 4. 固件文件在
# build/zephyr/zmk.uf2
```

## USB 刷入固件

Nice!Nano 使用 **UF2 Bootloader**，刷固件非常简单，像拷贝文件一样：

### 步骤

1. **进入 Bootloader 模式**：
   - 用 USB Type-C 线连接 Nice!Nano 到电脑
   - **快速双击** Nice!Nano 板子上的 **RST (Reset) 按钮**
   - 成功后，板子上的 LED 会闪烁，电脑上会出现一个名为 `NICENANO` 的 USB 存储设备

2. **复制固件文件**：
   - macOS:
     ```bash
     cp cyberfly-nice_nano_nrf52840.uf2 /Volumes/NICENANO/
     ```
   - Linux:
     ```bash
     cp cyberfly-nice_nano_nrf52840.uf2 /run/media/$USER/NICENANO/
     ```
   - Windows:
     - 打开文件管理器，找到 `NICENANO` 驱动器
     - 将 `.uf2` 文件拖放到该驱动器中

3. **等待刷写完成**：
   - 复制完成后，Nice!Nano 会自动重启并运行新固件
   - `NICENANO` 驱动器会自动消失，这是正常的

### 常见问题

- **没有出现 NICENANO 驱动器**：确保双击 RST 按钮的速度足够快（间隔约 0.5 秒），如果失败多试几次
- **想重新刷固件**：随时可以再次双击 RST 进入 Bootloader 模式
- **想恢复出厂**：刷入 ZMK 的 `settings_reset` 固件可以清除所有蓝牙配对和设置

## 蓝牙配对

刷入固件后，键盘会自动进入蓝牙广播模式：

1. 在电脑/手机的蓝牙设置中搜索 `CyberFly`
2. 点击配对即可使用
3. 支持同时配对 5 个设备，通过 BT 层 (需要自定义组合键进入) 切换

## 自定义键位

编辑 `app/boards/shields/cyberfly/cyberfly.keymap` 文件来修改键位映射，然后重新构建固件。

参考 ZMK 键位文档：https://zmk.dev/docs/keymaps
