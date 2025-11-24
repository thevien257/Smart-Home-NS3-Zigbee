# ZigBee Indoor Smart Home Network Simulation

## Mô tả

Dự án mô phỏng mạng ZigBee cho môi trường Smart Home trong nhà sử dụng NS-3.46. Phân tích ảnh hưởng của:
- Gaussian Noise (AWGN)
- Rayleigh Fading (multipath indoor)
- Khoảng cách giữa các node (5-20m)
- Số lượng node (4-10 devices)

**Topology**: Many-to-One Routing (Convergecast) - tất cả nodes route về Coordinator

## Kiến trúc hệ thống

### Network Topology
```
Grid Layout 3x2 (RowFirst)

Row 0:  [Node 0]---10m---[Node 1]---10m---[Node 2]
         (Coord)           (Router)         (Router)
           |                                    
          10m                                   
           |                                    
Row 1:  [Node 3]---10m---[Node 4]---10m---[Node 5]
        (Router)          (Router)         (Sensor)

→ All nodes route to Node 0 (Many-to-One)
→ Active data flow: Node 5 → Node 0
```

### Vai trò các Node
- **Node 0 (Coordinator)**: Trung tâm mạng, sink node
- **Node 1-4 (Router)**: Có khả năng routing và relay
- **Node 5 (Sensor)**: Thiết bị cảm biến gửi dữ liệu

## Yêu cầu hệ thống

### Phần mềm
- **NS-3.46** (standalone version)
- **Python 3.8+**
- **GCC/G++** (C++17 support)
- **CMake** (version 3.10+)
- **Git**

### Thư viện Python
```bash
pip install pandas matplotlib numpy seaborn
```

### NS-3 Modules cần thiết
- `core`
- `network`
- `lr-wpan`
- `zigbee` (contrib module)
- `mobility`
- `netanim`

## Cài đặt

### Bước 1: Cài đặt NS-3.46 (standalone)

```bash
# Clone NS-3 từ GitLab
cd ~
git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3.46
cd ns-3.46
git checkout ns-3.46

# Configure và build NS-3
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### Bước 2: Cài đặt ZigBee module

```bash
# Clone ZigBee module vào thư mục contrib
cd ~/ns-3.46/contrib
git clone <zigbee-module-repo> zigbee

# Rebuild NS-3 với module mới
cd ~/ns-3.46
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### Bước 3: Sao chép files vào NS-3

```bash
# Copy file simulation vào examples của zigbee module
cd ~/ns-3.46
cp zigbee-extended-sim.cc src/zigbee/examples/

# Copy scripts vào thư mục gốc NS-3
cp simulate_zigbee.sh ~/ns-3.46/
cp plot_extended_results.py ~/ns-3.46/

# Cấp quyền thực thi cho script
chmod +x ~/ns-3.46/simulate_zigbee.sh
```

### Bước 4: Chỉnh sửa CMakeLists.txt

Mở file `src/zigbee/examples/CMakeLists.txt` và thêm:

```cmake
set(base_examples
    zigbee-nwk-direct-join
    zigbee-nwk-association-join
    zigbee-nwk-routing
    zigbee-nwk-routing-grid
    zigbee-aps-data
    zigbee-extended-sim
)
foreach(
  example
  ${base_examples}
)
  build_lib_example(
    NAME ${example}
    SOURCE_FILES ${example}.cc
    LIBRARIES_TO_LINK ${libzigbee}
                      ${liblr-wpan}
                      ${libnetanim}
  )
endforeach()
```

### Bước 5: Build lại NS-3

```bash
cd ~/ns-3.46
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### Kiểm tra cài đặt

```bash
# Kiểm tra NS-3 version
./ns3 --version

# Test chạy simulation
./ns3 run zigbee-extended-sim
```

## Sử dụng

### Chạy simulation đơn lẻ

```bash
# Chạy với tham số mặc định (6 nodes, 10m, noise+fading)
./ns3 run zigbee-extended-sim

# Chạy với tham số tùy chỉnh
./ns3 run "zigbee-extended-sim --nodes=8 --distance=15 --packets=100"
```

### Tham số dòng lệnh

| Tham số | Mô tả | Giá trị mặc định | Phạm vi |
|---------|-------|------------------|---------|
| --nodes | Số lượng node | 6 | 4-10 |
| --distance | Khoảng cách giữa nodes (m) | 10 | 5-20 |
| --time | Thời gian simulation (s) | 120 | 60-300 |
| --packets | Số lượng gói tin gửi | 50 | 10-200 |
| --interval | Khoảng cách giữa các gói (s) | 2.0 | 0.5-5.0 |
| --noise | Bật Gaussian noise | true | true/false |
| --fading | Bật Rayleigh fading | true | true/false |
| --pathLossExp | Path loss exponent | 3.0 | 2.0-4.0 |
| --scenario | Tên kịch bản | Auto | string |
| --csv | File CSV output | zigbee_extended_results.csv | string |

### Ví dụ chạy

```bash
# Môi trường indoor nhẹ (ít vật cản)
./ns3 run "zigbee-extended-sim --pathLossExp=2.5 --distance=15"

# Môi trường indoor nặng (nhiều tường)
./ns3 run "zigbee-extended-sim --pathLossExp=3.5 --distance=10"

# Tắt noise và fading (ideal channel)
./ns3 run "zigbee-extended-sim --noise=false --fading=false"

# Mạng lớn với nhiều nodes
./ns3 run "zigbee-extended-sim --nodes=10 --distance=8 --packets=100"
```

## Chạy simulation hàng loạt

### Sử dụng Bash script

```bash
cd ~/ns-3.46
./simulate_zigbee.sh
```

Script này sẽ:
1. Chạy tất cả combinations của:
   - Distance: 5m, 10m, 15m, 20m
   - Nodes: 4, 6, 8, 10
   - Noise: ON/OFF
   - Fading: ON/OFF
2. Tổng cộng: **64 kịch bản**
3. Xuất kết quả ra `results_extended/zigbee_extended_results.csv`
4. Tự động tạo biểu đồ phân tích

### Nội dung file simulate_zigbee.sh

```bash
#!/bin/bash

# ZigBee Indoor Smart Home Simulation Script
# NS-3.46 - Standalone version

# Tạo thư mục kết quả
mkdir -p results_extended

# Tham số khảo sát
DISTANCES=(5 10 15 20)
NUM_NODES=(4 6 8 10)
NOISE_OPTS=(true false)
FADING_OPTS=(true false)

echo "=========================================="
echo "ZigBee Indoor Smart Home Simulation (NS-3.46)"
echo "Khảo sát: Distance, Number of Nodes, Noise, Fading"
echo "=========================================="

# Đếm tổng số lần chạy
TOTAL=$((${#DISTANCES[@]} * ${#NUM_NODES[@]} * ${#NOISE_OPTS[@]} * ${#FADING_OPTS[@]}))
COUNT=0

echo "Tổng số lần chạy: $TOTAL"
echo ""

# Loop qua tất cả combinations
for DIST in "${DISTANCES[@]}"; do
  for NODES in "${NUM_NODES[@]}"; do
    for NOISE in "${NOISE_OPTS[@]}"; do
      for FADING in "${FADING_OPTS[@]}"; do
        COUNT=$((COUNT + 1))
        
        # Tạo tên scenario
        SCENARIO="D${DIST}_N${NODES}"
        if [ "$NOISE" = "true" ]; then
          SCENARIO="${SCENARIO}_Noise"
        fi
        if [ "$FADING" = "true" ]; then
          SCENARIO="${SCENARIO}_Fading"
        fi
        
        echo "[$COUNT/$TOTAL] Running: $SCENARIO"
        echo "  Distance: ${DIST}m"
        echo "  Nodes: $NODES"
        echo "  Noise: $NOISE"
        echo "  Fading: $FADING"
        
        # Chạy simulation
        ./ns3 run "zigbee-extended-sim --nodes=$NODES --distance=$DIST --noise=$NOISE --fading=$FADING --scenario=$SCENARIO --csv=results_extended/zigbee_extended_results.csv"
        
        echo ""
      done
    done
  done
done

echo "=========================================="
echo "Hoàn thành tất cả $TOTAL kịch bản!"
echo "Kết quả được lưu tại: results_extended/zigbee_extended_results.csv"
echo "=========================================="
echo ""
echo "Đang tạo biểu đồ phân tích..."
python3 plot_extended_results.py

echo "Hoàn tất! Kiểm tra thư mục results_extended/"
```

### Kết quả mong đợi

```
==========================================
ZigBee Indoor Smart Home Simulation (NS-3.46)
Khảo sát: Distance, Number of Nodes, Noise, Fading
==========================================
Tổng số lần chạy: 64

[1/64] Running: D5_N4_Noise_Fading
  Distance: 5m
  Nodes: 4
  Noise: true
  Fading: true
...
Hoàn thành tất cả 64 kịch bản!
```

## Phân tích kết quả

### Vẽ biểu đồ

```bash
cd ~/ns-3.46
python3 plot_extended_results.py
```

### Biểu đồ được tạo

1. **analysis_distance.png**: Ảnh hưởng của khoảng cách
   - PDR vs Distance
   - SNR vs Distance
   - Delay vs Distance
   - Heatmap PDR

2. **analysis_num_nodes.png**: Ảnh hưởng của số lượng node
   - PDR vs Number of Nodes
   - SNR vs Number of Nodes
   - Delay vs Number of Nodes
   - Heatmap PDR

3. **analysis_matrix.png**: Ma trận tổng hợp
   - PDR cho 4 điều kiện: Ideal, Noise Only, Fading Only, Realistic

4. **analysis_impact.png**: Phân tích tác động
   - Ảnh hưởng riêng của Noise
   - Ảnh hưởng riêng của Fading
   - Tác động kết hợp
   - So sánh PDR trung bình

### Xem visualization NetAnim

```bash
netanim zigbee-indoor.xml
```

## Cấu trúc dữ liệu CSV

File `zigbee_extended_results.csv` chứa các cột:

| Cột | Mô tả |
|-----|-------|
| Scenario | Tên kịch bản |
| Distance | Khoảng cách giữa nodes (m) |
| NumNodes | Số lượng nodes |
| Noise | Có bật Gaussian noise (0/1) |
| Fading | Có bật Rayleigh fading (0/1) |
| Sent | Tổng số gói gửi |
| Received | Tổng số gói nhận |
| Dropped | Tổng số gói mất |
| DroppedNoise | Số gói mất do noise |
| DroppedFading | Số gói mất do fading |
| DroppedSensitivity | Số gói mất do sensitivity |
| PDR | Packet Delivery Ratio (%) |
| AvgSNR | SNR trung bình (dB) |
| MinSNR | SNR tối thiểu (dB) |
| MaxSNR | SNR tối đa (dB) |
| AvgDelay | Delay trung bình (ms) |

## Tham số kênh truyền

### Channel Model (Indoor Optimized)

```cpp
// Transmitter
TX Power: +4 dBm (2.5 mW) - Typical ZigBee CC2530/CC2652

// Path Loss Model
Reference Distance: 1.0m
Reference Path Loss: 40.77 dB @ 1m (2.4 GHz)
Path Loss Exponent: 3.0 (indoor with obstacles)
  - 2.0: Free space
  - 2.5-3.0: Light indoor
  - 3.5-4.0: Heavy indoor

// Gaussian Noise (AWGN)
Noise Floor: -174 dBm/Hz (thermal noise)
Noise Figure: 3 dB (typical ZigBee receiver)
Effective Noise: -171 dBm/Hz

// Rayleigh Fading
Distribution: Rayleigh (indoor multipath)
E[h²] = 1 (normalized)

// Receiver
Sensitivity: -97 dBm (CC2530 spec)
SNR Threshold: 3 dB (O-QPSK with DSSS)
```

## Cấu trúc thư mục

```
ns-3.46/
├── src/
│   └── zigbee/
│       ├── examples/
│       │   ├── zigbee-extended-sim.cc    # Main simulation code
│       │   └── CMakeLists.txt            # Build configuration (MODIFIED)
│       ├── model/
│       └── helper/
├── simulate_zigbee.sh                     # Batch simulation script
├── plot_extended_results.py               # Data analysis & plotting
├── results_extended/                      # Output directory (auto-created)
│   ├── zigbee_extended_results.csv
│   ├── analysis_distance.png
│   ├── analysis_num_nodes.png
│   ├── analysis_matrix.png
│   └── analysis_impact.png
└── zigbee-indoor.xml                      # NetAnim visualization file
```

## Troubleshooting

### Lỗi: Module zigbee not found

```bash
# Kiểm tra module có tồn tại
ls ~/ns-3.46/contrib/zigbee

# Rebuild NS-3
cd ~/ns-3.46
./ns3 clean
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### Lỗi: zigbee-extended-sim not found

```bash
# Kiểm tra file có đúng vị trí
ls ~/ns-3.46/src/zigbee/examples/zigbee-extended-sim.cc

# Kiểm tra CMakeLists.txt đã được cập nhật
cat ~/ns-3.46/src/zigbee/examples/CMakeLists.txt | grep zigbee-extended-sim

# Rebuild
cd ~/ns-3.46
./ns3 build
```

### Lỗi: Python ModuleNotFoundError

```bash
# Cài đặt thư viện thiếu
pip3 install pandas matplotlib numpy seaborn
```

### Lỗi: Permission denied cho script

```bash
chmod +x ~/ns-3.46/simulate_zigbee.sh
```

### Simulation chạy chậm

```bash
# Giảm số packets hoặc simulation time
./ns3 run "zigbee-extended-sim --packets=20 --time=60"

# Build với optimization
./ns3 configure --build-profile=optimized
./ns3 build
```

### Lỗi: Build failed

```bash
# Kiểm tra dependencies
sudo apt-get update
sudo apt-get install g++ python3 python3-dev pkg-config sqlite3 \
  libsqlite3-dev cmake python3-setuptools git qtbase5-dev qtchooser \
  qt5-qmake qtbase5-dev-tools

# Clean và rebuild
./ns3 clean
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

