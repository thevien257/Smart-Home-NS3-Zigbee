# Hướng dẫn Smart Home ZigBee Performance Simulation
## Phiên bản nâng cao với Channel Effects & Metrics

---

## 🎯 Tổng quan

Chương trình mô phỏng mạng ZigBee nhà thông minh với:

### Tính năng chính
- ✅ **Gaussian Noise (AWGN)** - Mô phỏng nhiễu
- ✅ **Rayleigh Fading** - Suy hao đa đường
- ✅ **Performance Metrics** - Đo throughput, delay, PDR
- ✅ **Power Consumption** - Ước lượng tiêu thụ năng lượng
- ✅ **Scalable Network** - Số nodes linh hoạt (3-30+)
- ✅ **CSV Export** - Xuất dữ liệu phân tích
- ✅ **NetAnim Visualization** - Trực quan hóa mạng

---

## 🚀 Biên dịch và chạy

### Bước 1: Biên dịch
```bash
cd ~/ns-3-dev
./ns3 configure --enable-examples
./ns3 build
```

### Bước 2: Chạy cơ bản
```bash
./ns3 run smart-home-zigbee-complete-ver3
```

---

## ⚙️ Tham số dòng lệnh

### 📊 Cấu hình mạng

#### `--numNodes` (Số lượng nodes)
```bash
# Mạng nhỏ - tối thiểu
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=3"

# Mạng vừa - khuyến nghị
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=10"

# Mạng lớn - test scalability
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=20"
```

**Lưu ý:** Tối thiểu 3 nodes (1 Coordinator + 2 Routers)

---

#### `--simTime` (Thời gian mô phỏng, giây)
```bash
# Test nhanh
./ns3 run "smart-home-zigbee-complete-ver3 --simTime=60"

# Chuẩn
./ns3 run "smart-home-zigbee-complete-ver3 --simTime=300"

# Dài hạn
./ns3 run "smart-home-zigbee-complete-ver3 --simTime=600"
```

---

### 📡 Cấu hình kênh truyền

#### `--enableNoise` (Bật nhiễu Gaussian)
```bash
# Có nhiễu (mặc định - realistic)
./ns3 run "smart-home-zigbee-complete-ver3 --enableNoise=true"

# Không nhiễu (ideal channel)
./ns3 run "smart-home-zigbee-complete-ver3 --enableNoise=false"
```

---

#### `--enableFading` (Bật Rayleigh fading)
```bash
# Có fading (mặc định - realistic)
./ns3 run "smart-home-zigbee-complete-ver3 --enableFading=true"

# Không fading
./ns3 run "smart-home-zigbee-complete-ver3 --enableFading=false"
```

---

#### `--snrThreshold` (Ngưỡng SNR, dB)
```bash
# Chặt chẽ - chất lượng cao
./ns3 run "smart-home-zigbee-complete-ver3 --snrThreshold=10.0"

# Mặc định
./ns3 run "smart-home-zigbee-complete-ver3 --snrThreshold=6.0"

# Lỏng lẻo - chấp nhận tín hiệu yếu
./ns3 run "smart-home-zigbee-complete-ver3 --snrThreshold=3.0"
```

---

### 🔀 Cấu hình routing

#### `--manyToOne` (Chế độ định tuyến)
```bash
# Many-to-One (mặc định - tối ưu cho sensor networks)
./ns3 run "smart-home-zigbee-complete-ver3 --manyToOne=true"

# Mesh routing (tối ưu cho mạng phân tán)
./ns3 run "smart-home-zigbee-complete-ver3 --manyToOne=false"
```

---

### 💾 Export dữ liệu

#### `--exportCSV` và `--csvFile`
```bash
# Xuất ra file mặc định
./ns3 run "smart-home-zigbee-complete-ver3 --exportCSV=true"

# Xuất ra file tùy chỉnh
./ns3 run "smart-home-zigbee-complete-ver3 --exportCSV=true --csvFile=my_test.csv"
```

---

### 🐛 Debug

#### `--verbose` (Log chi tiết)
```bash
./ns3 run "smart-home-zigbee-complete-ver3 --verbose=true"
```

---

## 🎨 Kết hợp tham số

### 📋 Các scenario thực tế

#### 1. Test kênh truyền lý tưởng
```bash
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=false \
  --enableFading=false \
  --simTime=300 \
  --exportCSV=true \
  --csvFile=ideal_channel.csv"
```

---

#### 2. Test kênh truyền khắc nghiệt
```bash
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=true \
  --enableFading=true \
  --snrThreshold=10.0 \
  --simTime=300 \
  --exportCSV=true \
  --csvFile=harsh_channel.csv"
```

---

#### 3. Test scalability (nhiều kích thước mạng)
```bash
# Bash script
for n in 3 5 8 10 15 20; do
  echo "Testing $n nodes..."
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --numNodes=$n \
    --simTime=300 \
    --exportCSV=true \
    --csvFile=scalability.csv"
done
```

---

#### 4. So sánh Mesh vs Many-to-One
```bash
# Mesh
./ns3 run "smart-home-zigbee-complete-ver3 \
  --manyToOne=false \
  --numNodes=10 \
  --exportCSV=true \
  --csvFile=mesh_results.csv"

# Many-to-One
./ns3 run "smart-home-zigbee-complete-ver3 \
  --manyToOne=true \
  --numNodes=10 \
  --exportCSV=true \
  --csvFile=mto_results.csv"
```

---

#### 5. Test ảnh hưởng SNR threshold
```bash
for snr in 3 6 9 12; do
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --snrThreshold=$snr \
    --exportCSV=true \
    --csvFile=snr_impact.csv"
done
```

---

## 📈 Đọc kết quả Output

### Terminal output gồm:

#### 1️⃣ **Packet Statistics**
```
--- Packet Statistics ---
  Packets Transmitted:  150
  Packets Received:     142
  Packets Failed:       8
  Bytes Transmitted:    300
  Bytes Received:       284
```

---

#### 2️⃣ **Delivery Performance**
```
--- Delivery Performance ---
  Packet Delivery Ratio: 94.67%
  Packet Loss Rate:      5.33%
```

**Đánh giá:**
- ✅ Tốt: PDR > 95%
- ⚠️ Chấp nhận: PDR 80-95%
- ❌ Kém: PDR < 80%

---

#### 3️⃣ **Channel Quality Statistics** ⭐ MỚI
```
--- Channel Quality Statistics ---
  Packets Dropped by Noise:  3
  Packets Dropped by Fading: 5
  Average SNR:           12.45 dB
  Min SNR:               4.23 dB
  Max SNR:               18.67 dB
  SNR Threshold:         6.00 dB
  Average Fading Coeff:  0.87
  Min Fading Coeff:      0.34
  Max Fading Coeff:      1.52
```

**Giải thích SNR:**
- 🟢 > 15 dB: Tín hiệu mạnh
- 🟡 10-15 dB: Tín hiệu tốt
- 🟠 6-10 dB: Tín hiệu chấp nhận
- 🔴 < 6 dB: Tín hiệu yếu, có thể mất gói

---

#### 4️⃣ **Throughput**
```
--- Throughput ---
  Data Duration:         285.450s
  Throughput:            7.93 kbps
  Throughput:            0.50 packets/s
  Average Packet Size:   2.0 bytes
```

**ZigBee Throughput:**
- Max lý thuyết: 250 kbps
- Thực tế tốt: 50-100 kbps
- Phụ thuộc: overhead, collisions, retransmissions

---

#### 5️⃣ **End-to-End Delay** ⭐ MỚI
```
--- End-to-End Delay ---
  Samples:               142
  Average Delay:         45.234 ms
  Minimum Delay:         12.456 ms
  Maximum Delay:         89.123 ms
```

**Đánh giá delay:**
- ✅ Tốt: < 100ms
- ⚠️ Chấp nhận: 100-500ms
- ❌ Kém: > 500ms

---

#### 6️⃣ **Power Consumption** ⭐ MỚI
```
--- Power Consumption (Estimated) ---
  TX Power:              12.456 mW·s (0.69%)
  RX Power:              8.234 mW·s (0.46%)
  Idle Power:            1779.31 mW·s (98.85%)
  Total Power:           1800.00 mW·s
  Average Power/Node:    300.00 mW·s
```

**Phân tích:**
- Idle chiếm > 95% → Cơ hội tiết kiệm năng lượng bằng sleep mode
- TX/RX < 5% → Hiệu quả cho sensor networks

---

#### 7️⃣ **Network Efficiency**
```
--- Network Efficiency ---
  Join Success Rate:     100.00%
  Route Discoveries:     5
  Group Commands:        3
```

---

## 📊 Phân tích file CSV

### Cấu trúc CSV

File CSV chứa 27 cột:
```
NumNodes, SimTime, PacketsSent, PacketsReceived, PacketsFailed,
BytesSent, BytesReceived, PDR, LossRate, ThroughputKbps,
AvgDelay, MinDelay, MaxDelay, TxPower, RxPower, IdlePower,
TotalPower, JoinSuccessRate, RouteDiscoveries,
PacketsDroppedNoise, PacketsDroppedFading, AvgSNR, MinSNR, MaxSNR,
AvgFading, MinFading, MaxFading
```

---

### Phân tích bằng Python

```python
import pandas as pd
import matplotlib.pyplot as plt

# Đọc CSV
df = pd.read_csv('zigbee_performance.csv')

# ===== 1. Biểu đồ PDR vs Số nodes =====
plt.figure(figsize=(10, 6))
plt.plot(df['NumNodes'], df['PDR'], marker='o', linewidth=2)
plt.xlabel('Number of Nodes', fontsize=12)
plt.ylabel('Packet Delivery Ratio (%)', fontsize=12)
plt.title('Network Scalability Analysis', fontsize=14)
plt.grid(True, alpha=0.3)
plt.savefig('pdr_scalability.png', dpi=300)
plt.show()

# ===== 2. Biểu đồ SNR =====
plt.figure(figsize=(10, 6))
plt.plot(df['NumNodes'], df['AvgSNR'], 
         marker='s', label='Average SNR', linewidth=2)
plt.axhline(y=6.0, color='r', linestyle='--', 
            label='SNR Threshold', linewidth=2)
plt.xlabel('Number of Nodes', fontsize=12)
plt.ylabel('SNR (dB)', fontsize=12)
plt.title('Signal Quality Analysis', fontsize=14)
plt.legend(fontsize=10)
plt.grid(True, alpha=0.3)
plt.savefig('snr_analysis.png', dpi=300)
plt.show()

# ===== 3. Biểu đồ Delay =====
plt.figure(figsize=(10, 6))
plt.plot(df['NumNodes'], df['AvgDelay'], 
         marker='^', color='green', linewidth=2)
plt.xlabel('Number of Nodes', fontsize=12)
plt.ylabel('Average Delay (ms)', fontsize=12)
plt.title('End-to-End Delay Analysis', fontsize=14)
plt.grid(True, alpha=0.3)
plt.savefig('delay_analysis.png', dpi=300)
plt.show()

# ===== 4. Biểu đồ phân bổ công suất =====
fig, ax = plt.subplots(figsize=(10, 6))
width = 0.25
x = range(len(df))

ax.bar([i - width for i in x], df['TxPower'], 
       width, label='TX Power')
ax.bar(x, df['RxPower'], 
       width, label='RX Power')
ax.bar([i + width for i in x], df['IdlePower'], 
       width, label='Idle Power')

ax.set_xlabel('Test Run', fontsize=12)
ax.set_ylabel('Power (mW·s)', fontsize=12)
ax.set_title('Power Consumption Distribution', fontsize=14)
ax.legend(fontsize=10)
plt.grid(True, alpha=0.3)
plt.savefig('power_distribution.png', dpi=300)
plt.show()

# ===== 5. So sánh packet loss =====
fig, ax = plt.subplots(figsize=(10, 6))
ax.bar(df['NumNodes'], df['PacketsDroppedNoise'], 
       label='Dropped by Noise', alpha=0.7)
ax.bar(df['NumNodes'], df['PacketsDroppedFading'], 
       bottom=df['PacketsDroppedNoise'],
       label='Dropped by Fading', alpha=0.7)

ax.set_xlabel('Number of Nodes', fontsize=12)
ax.set_ylabel('Packets Dropped', fontsize=12)
ax.set_title('Packet Loss Analysis by Cause', fontsize=14)
ax.legend(fontsize=10)
plt.grid(True, alpha=0.3)
plt.savefig('loss_causes.png', dpi=300)
plt.show()

# ===== 6. Thống kê tổng hợp =====
print("\n=== Summary Statistics ===")
print(f"Average PDR: {df['PDR'].mean():.2f}%")
print(f"Average Throughput: {df['ThroughputKbps'].mean():.2f} kbps")
print(f"Average Delay: {df['AvgDelay'].mean():.2f} ms")
print(f"Average SNR: {df['AvgSNR'].mean():.2f} dB")
```

---

### Phân tích bằng Excel

1. Mở file CSV trong Excel
2. Chèn Pivot Table
3. Tạo biểu đồ từ các cột:
   - PDR vs NumNodes
   - Throughput vs NumNodes
   - Delay vs NumNodes
   - SNR vs NumNodes

---

## 🎬 NetAnim Visualization

### Chạy NetAnim
```bash
# File XML tự động tạo
netanim zigbee-network-with-noise.xml
```

### Màu sắc nodes

| Màu | Loại node | Mô tả |
|-----|-----------|-------|
| 🔴 Đỏ | Coordinator | Điều phối mạng |
| 🔵 Xanh dương | Router | Định tuyến |
| 🟢 Xanh lá | Sensor | Cảm biến |
| 🟡 Vàng | Light | Đèn điều khiển |

---

## 🧪 Các thí nghiệm quan trọng

### Experiment 1: Ảnh hưởng của Noise

```bash
# Không có noise
./ns3 run "smart-home-zigbee-complete-ver3 \
  --enableNoise=false \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=exp1_no_noise.csv"

# Có noise
./ns3 run "smart-home-zigbee-complete-ver3 \
  --enableNoise=true \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=exp1_with_noise.csv"
```

**So sánh:** PDR, packets dropped by noise

---

### Experiment 2: Ảnh hưởng của Fading

```bash
# Không có fading
./ns3 run "smart-home-zigbee-complete-ver3 \
  --enableNoise=false \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=exp2_no_fading.csv"

# Có fading
./ns3 run "smart-home-zigbee-complete-ver3 \
  --enableNoise=false \
  --enableFading=true \
  --exportCSV=true \
  --csvFile=exp2_with_fading.csv"
```

**So sánh:** PDR, fading coefficients, packets dropped by fading

---

### Experiment 3: Scalability Test

```bash
#!/bin/bash
for nodes in 3 5 8 10 15 20 25; do
  echo "=== Testing with $nodes nodes ==="
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --numNodes=$nodes \
    --simTime=300 \
    --exportCSV=true \
    --csvFile=exp3_scalability.csv"
  sleep 5
done
```

**Phân tích:** PDR, throughput, delay vs number of nodes

---

### Experiment 4: SNR Threshold Sensitivity

```bash
#!/bin/bash
for snr in 3.0 6.0 9.0 12.0 15.0; do
  echo "=== Testing SNR threshold: $snr dB ==="
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --snrThreshold=$snr \
    --exportCSV=true \
    --csvFile=exp4_snr_threshold.csv"
  sleep 5
done
```

**Phân tích:** Packet loss rate vs SNR threshold

---

### Experiment 5: Routing Comparison

```bash
# Many-to-One
./ns3 run "smart-home-zigbee-complete-ver3 \
  --manyToOne=true \
  --numNodes=15 \
  --simTime=600 \
  --exportCSV=true \
  --csvFile=exp5_mto.csv"

# Mesh
./ns3 run "smart-home-zigbee-complete-ver3 \
  --manyToOne=false \
  --numNodes=15 \
  --simTime=600 \
  --exportCSV=true \
  --csvFile=exp5_mesh.csv"
```

**So sánh:** Route discoveries, delay, PDR

---

## 🔧 Troubleshooting

### Vấn đề: PDR quá thấp (< 80%)

**Nguyên nhân có thể:**
- SNR threshold quá cao
- Nhiễu/fading quá mạnh
- Mạng quá lớn

**Giải pháp:**
```bash
# Giảm SNR threshold
./ns3 run "smart-home-zigbee-complete-ver3 --snrThreshold=3.0"

# Tắt noise/fading để test
./ns3 run "smart-home-zigbee-complete-ver3 \
  --enableNoise=false --enableFading=false"

# Giảm số nodes
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=5"
```

---

### Vấn đề: Delay quá cao (> 500ms)

**Giải pháp:**
```bash
# Bật Many-to-One routing
./ns3 run "smart-home-zigbee-complete-ver3 --manyToOne=true"

# Giảm số nodes
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=8"
```

---

### Vấn đề: Throughput thấp

**Kiểm tra:**
- Packet loss rate
- Average packet size
- Number of retransmissions

**Debug:**
```bash
./ns3 run "smart-home-zigbee-complete-ver3 --verbose=true" \
  | grep -E "TRANSMITTED|RECEIVED|FAILED"
```

---

### Vấn đề: Join failures

**Debug:**
```bash
NS_LOG="ZigbeeNwk=level_debug" \
  ./ns3 run smart-home-zigbee-complete-ver3
```

---

## 📚 Tham số kênh truyền (trong code)

Các tham số có thể chỉnh sửa:

```cpp
// Noise và tín hiệu
const double NOISE_FLOOR_DBM = -95.0;      // Ngưỡng nhiễu
const double TX_POWER_DBM = 0.0;           // Công suất phát (1 mW)
const double SNR_THRESHOLD_DB = 6.0;       // SNR tối thiểu

// Path loss
const double PATH_LOSS_EXPONENT = 3.0;     // Indoor: 3-4, Outdoor: 2-3
const double REFERENCE_DISTANCE = 1.0;     // Khoảng cách tham chiếu

// Công suất
const double TX_POWER = 35.0;              // mW
const double RX_POWER = 25.0;              // mW
const double IDLE_POWER = 0.3;             // mW
```

---

## 📖 Các metrics quan trọng

### 1. Packet Delivery Ratio (PDR)
```
PDR = (Packets Received / Packets Transmitted) × 100%
```
- Đo độ tin cậy của mạng
- Mục tiêu: > 95%

### 2. Packet Loss Rate
```
Loss Rate = 100% - PDR
```
- Tỷ lệ gói tin bị mất

### 3. Throughput
```
Throughput = (Bytes Received × 8) / Duration (kbps)
```
- Đo hiệu suất truyền dữ liệu

### 4. End-to-End Delay
```
Delay = Time Received - Time Sent
```
- Đo độ trễ truyền tin
- Quan trọng cho ứng dụng real-time

### 5. Signal-to-Noise Ratio (SNR)
```
SNR (dB) = Signal Power (dBm) - Noise Power (dBm)
```
- Đo chất lượng tín hiệu

### 6. Power Consumption
```
Total Power = TX Power + RX Power + Idle Power
```
- Ước lượng tiêu thụ năng lượng

---

## 💡 Tips và Best Practices

### 1. Chạy nhiều lần với seed khác nhau
```bash
for seed in 1 2 3 4 5; do
  # Chỉnh RngSeedManager::SetRun($seed) trong code
  ./ns3 run smart-home-zigbee-complete-ver3
done
```

### 2. Sử dụng script automation
```bash
#!/bin/bash
# automated_tests.sh

TESTS=(
  "--numNodes=5 --snrThreshold=6.0"
  "--numNodes=10 --snrThreshold=6.0"
  "--numNodes=15 --snrThreshold=6.0"
  "--numNodes=20 --snrThreshold=6.0"
)

for test in "${TESTS[@]}"; do
  echo "Running: $test"
  ./ns3 run "smart-home-zigbee-complete-ver3 $test \
    --exportCSV=true --csvFile=results.csv"
done
```

### 3. Lưu log files
```bash
./ns3 run smart-home-zigbee-complete-ver3 \
  > output_$(date +%Y%m%d_%H%M%S).log 2>&1
```

### 4. So sánh kết quả
```bash
# Chạy baseline
./ns3 run "smart-home-zigbee-complete-ver3 \
  --exportCSV=true --csvFile=baseline.csv"

# Chạy với thay đổi
./ns3 run "smart-home-zigbee-complete-ver3 \
  --snrThreshold=10.0 \
  --exportCSV=true --csvFile=modified.csv"

# So sánh bằng Python
python3 compare_results.py baseline.csv modified.csv
```

---

## 🎓 Ví dụ nghiên cứu hoàn chỉnh

### Mục tiêu: Nghiên cứu ảnh hưởng của kích thước mạng

```bash
#!/bin/bash
# research_scalability.sh

OUTPUT_DIR="results_$(date +%Y%m%d)"
mkdir -p $OUTPUT_DIR

for nodes in 3 5 8 10 12 15 18 20; do
  echo "========================================="
  echo "Testing with $nodes nodes"
  echo "========================================="
  
  for run in {1..5}; do
    echo "  Run $run/5..."
    ./ns3 run "smart-home-zigbee-complete-ver3 \
      --numNodes=$nodes \
      --simTime=300 \
      --exportCSV=true \
      --csvFile=$OUTPUT_DIR/scalability_results.csv" \
      > $OUTPUT_DIR/log_${nodes}nodes_run${run}.txt 2>&1
    
    sleep 3
  done
done

echo "All tests completed!"
echo "Results saved in: $OUTPUT_DIR"

# Tạo báo cáo
python3 << EOF
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('$OUTPUT_DIR/scalability_results.csv')

# Group by NumNodes và tính trung bình
grouped = df.groupby('NumNodes').mean()

# Vẽ các biểu đồ
fig, axes = plt.subplots(2, 2, figsize=(15, 12))

# PDR
axes[0, 0].plot(grouped.index, grouped['PDR'], marker='o')
axes[0, 0].set_title('Packet Delivery Ratio vs Network Size')
axes[0, 0].set_xlabel('Number of Nodes')
axes[0, 0].set_ylabel('PDR (%)')
axes[0, 0].grid(True)

# Throughput
axes[0, 1].plot(grouped.index, grouped['ThroughputKbps'], marker='s')
axes[0, 1].set_title('Throughput vs Network Size')
axes[0, 1].set_xlabel('Number of Nodes')
axes[0, 1].set_ylabel('Throughput (kbps)')
axes[0, 1].grid(True)

# Delay
axes[1, 0].plot(grouped.index, grouped['AvgDelay'], marker='^')
axes[1, 0].set_title('Average Delay vs Network Size')
axes[1, 0].set_xlabel('Number of Nodes')
axes[1, 0].set_ylabel('Delay (ms)')
axes[1, 0].grid(True)

# SNR
axes[1, 1].plot(grouped.index, grouped['AvgSNR'], marker='d')
axes[1, 1].axhline(y=6.0, color='r', linestyle='--', label='Threshold')
axes[1, 1].set_title('Average SNR vs Network Size')
axes[1, 1].set_xlabel('Number of Nodes')
axes[1, 1].set_ylabel('SNR (dB)')
axes[1, 1].legend()
axes[1, 1].grid(True)

plt.tight_layout()
plt.savefig('$OUTPUT_DIR/scalability_analysis.png', dpi=300)
print("Report generated: $OUTPUT_DIR/scalability_analysis.png")
EOF
```

---

## 🌟 Kết luận

### Ưu điểm của phiên bản nâng cao:
✅ Realistic channel modeling (noise + fading)
✅ Comprehensive performance metrics
✅ Flexible scalability testing
✅ Data export for further analysis
✅ Visual network animation
✅ Power consumption estimation

### Ứng dụng:
- 📊 Nghiên cứu hiệu năng mạng ZigBee
- 🔬 Phân tích ảnh hưởng của kênh truyền
- 📈 Đánh giá scalability
- 🔋 Tối ưu hóa năng lượng
- 📚 Giáo dục và đào tạo

---

## 📞 Hỗ trợ thêm

Nếu cần hỗ trợ:
1. Kiểm tra NS-3 documentation
2. Xem NS-3 tutorials
3. Tham khảo ZigBee specification
4. Kiểm tra log files với `--verbose=true`

---

**Chúc bạn thành công với mô phỏng ZigBee!** 🚀

---

## 📋 Quick Reference - Cheat Sheet

### Lệnh cơ bản nhất
```bash
# Chạy với cấu hình mặc định
./ns3 run smart-home-zigbee-complete-ver3
```

### Lệnh thường dùng
```bash
# Test nhanh
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=5 --simTime=60"

# Test đầy đủ + xuất CSV
./ns3 run "smart-home-zigbee-complete-ver3 --numNodes=10 --simTime=300 --exportCSV=true"

# Test kênh lý tưởng
./ns3 run "smart-home-zigbee-complete-ver3 --enableNoise=false --enableFading=false"

# Test kênh khắc nghiệt
./ns3 run "smart-home-zigbee-complete-ver3 --enableNoise=true --enableFading=true --snrThreshold=10.0"

# Debug mode
./ns3 run "smart-home-zigbee-complete-ver3 --verbose=true"
```

---

## 🎯 Các giá trị tham số đề xuất

### Cho mạng nhà thông minh nhỏ (< 10 thiết bị)
```bash
--numNodes=6 --simTime=300 --snrThreshold=6.0
```

### Cho mạng nhà thông minh vừa (10-20 thiết bị)
```bash
--numNodes=15 --simTime=600 --snrThreshold=6.0 --manyToOne=true
```

### Cho mạng nhà thông minh lớn (> 20 thiết bị)
```bash
--numNodes=25 --simTime=900 --snrThreshold=6.0 --manyToOne=true
```

### Cho môi trường nhiễu cao (gần WiFi, Bluetooth)
```bash
--enableNoise=true --snrThreshold=9.0
```

### Cho môi trường có vật cản nhiều
```bash
--enableFading=true --snrThreshold=6.0
```

---

## 📊 Bảng đánh giá nhanh

### Packet Delivery Ratio (PDR)
| PDR | Đánh giá | Hành động |
|-----|----------|-----------|
| > 98% | Xuất sắc | Tốt |
| 95-98% | Tốt | OK |
| 90-95% | Chấp nhận | Xem xét cải thiện |
| 85-90% | Khá kém | Cần cải thiện |
| < 85% | Kém | Kiểm tra lại cấu hình |

### Average Delay
| Delay | Đánh giá | Ứng dụng |
|-------|----------|----------|
| < 50ms | Xuất sắc | Real-time control |
| 50-100ms | Tốt | Interactive apps |
| 100-300ms | Chấp nhận | Monitoring |
| 300-500ms | Khá kém | Background tasks |
| > 500ms | Kém | Không phù hợp |

### SNR
| SNR (dB) | Chất lượng | Link Status |
|----------|------------|-------------|
| > 20 | Xuất sắc | Very strong |
| 15-20 | Rất tốt | Strong |
| 10-15 | Tốt | Good |
| 6-10 | Chấp nhận | Fair |
| 3-6 | Yếu | Marginal |
| < 3 | Rất yếu | Poor |

---

## 🔍 Advanced Analysis Commands

### 1. Extract specific metrics from log
```bash
# Extract PDR values
./ns3 run smart-home-zigbee-complete-ver3 2>&1 | grep "Packet Delivery Ratio"

# Extract SNR statistics
./ns3 run smart-home-zigbee-complete-ver3 2>&1 | grep "SNR"

# Extract delay information
./ns3 run smart-home-zigbee-complete-ver3 2>&1 | grep "Delay"
```

### 2. Run batch tests and save results
```bash
#!/bin/bash
# batch_test.sh

DATE=$(date +%Y%m%d_%H%M%S)
RESULTS_DIR="batch_results_$DATE"
mkdir -p $RESULTS_DIR

CONFIGS=(
  "3:300:6.0:true:true"
  "5:300:6.0:true:true"
  "8:300:6.0:true:true"
  "10:300:6.0:true:true"
  "15:300:6.0:true:true"
)

for config in "${CONFIGS[@]}"; do
  IFS=':' read -r nodes time snr noise fading <<< "$config"
  
  echo "Testing: nodes=$nodes, time=$time, snr=$snr"
  
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --numNodes=$nodes \
    --simTime=$time \
    --snrThreshold=$snr \
    --enableNoise=$noise \
    --enableFading=$fading \
    --exportCSV=true \
    --csvFile=$RESULTS_DIR/results.csv" \
    > $RESULTS_DIR/log_${nodes}nodes.txt 2>&1
    
  echo "Completed: $nodes nodes"
  sleep 5
done

echo "All tests completed!"
echo "Results in: $RESULTS_DIR"
```

### 3. Compare two configurations
```bash
#!/bin/bash
# compare_configs.sh

echo "Running Configuration A (Ideal)..."
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=false \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=config_a.csv" > log_a.txt 2>&1

echo "Running Configuration B (Realistic)..."
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=true \
  --enableFading=true \
  --exportCSV=true \
  --csvFile=config_b.csv" > log_b.txt 2>&1

echo "Comparison:"
echo "========================================="
echo "Config A (Ideal Channel):"
grep "Packet Delivery Ratio" log_a.txt
grep "Average Delay" log_a.txt
echo ""
echo "Config B (Realistic Channel):"
grep "Packet Delivery Ratio" log_b.txt
grep "Average Delay" log_b.txt
```

---

## 📈 Visualization Scripts

### Python script for comprehensive analysis
```python
#!/usr/bin/env python3
# analyze_results.py

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def analyze_zigbee_results(csv_file):
    """Comprehensive analysis of ZigBee simulation results"""
    
    # Read data
    df = pd.read_csv(csv_file)
    
    # Create figure with subplots
    fig = plt.figure(figsize=(16, 12))
    
    # 1. PDR Analysis
    ax1 = plt.subplot(3, 3, 1)
    ax1.plot(df['NumNodes'], df['PDR'], 'o-', linewidth=2, markersize=8)
    ax1.axhline(y=95, color='g', linestyle='--', label='Target (95%)')
    ax1.set_xlabel('Number of Nodes')
    ax1.set_ylabel('PDR (%)')
    ax1.set_title('Packet Delivery Ratio')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Throughput Analysis
    ax2 = plt.subplot(3, 3, 2)
    ax2.plot(df['NumNodes'], df['ThroughputKbps'], 's-', 
             linewidth=2, markersize=8, color='orange')
    ax2.set_xlabel('Number of Nodes')
    ax2.set_ylabel('Throughput (kbps)')
    ax2.set_title('Network Throughput')
    ax2.grid(True, alpha=0.3)
    
    # 3. Delay Analysis
    ax3 = plt.subplot(3, 3, 3)
    ax3.plot(df['NumNodes'], df['AvgDelay'], '^-', 
             linewidth=2, markersize=8, color='red')
    ax3.axhline(y=100, color='orange', linestyle='--', label='Target (100ms)')
    ax3.set_xlabel('Number of Nodes')
    ax3.set_ylabel('Delay (ms)')
    ax3.set_title('End-to-End Delay')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    
    # 4. SNR Distribution
    ax4 = plt.subplot(3, 3, 4)
    ax4.errorbar(df['NumNodes'], df['AvgSNR'], 
                 yerr=[df['AvgSNR']-df['MinSNR'], df['MaxSNR']-df['AvgSNR']],
                 fmt='o-', linewidth=2, markersize=8, color='purple', capsize=5)
    ax4.axhline(y=6, color='r', linestyle='--', label='Threshold (6 dB)')
    ax4.set_xlabel('Number of Nodes')
    ax4.set_ylabel('SNR (dB)')
    ax4.set_title('Signal-to-Noise Ratio')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    
    # 5. Packet Loss Breakdown
    ax5 = plt.subplot(3, 3, 5)
    width = 0.35
    x = np.arange(len(df))
    ax5.bar(x - width/2, df['PacketsDroppedNoise'], width, 
            label='Noise', alpha=0.8)
    ax5.bar(x + width/2, df['PacketsDroppedFading'], width, 
            label='Fading', alpha=0.8)
    ax5.set_xlabel('Test Case')
    ax5.set_ylabel('Packets Dropped')
    ax5.set_title('Packet Loss by Cause')
    ax5.legend()
    ax5.grid(True, alpha=0.3)
    
    # 6. Power Consumption
    ax6 = plt.subplot(3, 3, 6)
    ax6.plot(df['NumNodes'], df['TotalPower']/df['NumNodes'], 
             'o-', linewidth=2, markersize=8, color='green')
    ax6.set_xlabel('Number of Nodes')
    ax6.set_ylabel('Power per Node (mW·s)')
    ax6.set_title('Average Power Consumption')
    ax6.grid(True, alpha=0.3)
    
    # 7. Power Distribution (Stacked)
    ax7 = plt.subplot(3, 3, 7)
    ax7.fill_between(range(len(df)), 0, df['TxPower'], 
                     label='TX', alpha=0.7)
    ax7.fill_between(range(len(df)), df['TxPower'], 
                     df['TxPower']+df['RxPower'], 
                     label='RX', alpha=0.7)
    ax7.fill_between(range(len(df)), df['TxPower']+df['RxPower'], 
                     df['TotalPower'], 
                     label='Idle', alpha=0.7)
    ax7.set_xlabel('Test Case')
    ax7.set_ylabel('Power (mW·s)')
    ax7.set_title('Power Distribution')
    ax7.legend()
    ax7.grid(True, alpha=0.3)
    
    # 8. Fading Statistics
    ax8 = plt.subplot(3, 3, 8)
    ax8.errorbar(df['NumNodes'], df['AvgFading'],
                 yerr=[df['AvgFading']-df['MinFading'], 
                       df['MaxFading']-df['AvgFading']],
                 fmt='d-', linewidth=2, markersize=8, 
                 color='brown', capsize=5)
    ax8.set_xlabel('Number of Nodes')
    ax8.set_ylabel('Fading Coefficient')
    ax8.set_title('Rayleigh Fading Analysis')
    ax8.grid(True, alpha=0.3)
    
    # 9. Loss Rate vs SNR
    ax9 = plt.subplot(3, 3, 9)
    scatter = ax9.scatter(df['AvgSNR'], df['LossRate'], 
                         c=df['NumNodes'], s=100, cmap='viridis')
    ax9.set_xlabel('Average SNR (dB)')
    ax9.set_ylabel('Loss Rate (%)')
    ax9.set_title('Loss Rate vs SNR')
    plt.colorbar(scatter, ax=ax9, label='Number of Nodes')
    ax9.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('zigbee_comprehensive_analysis.png', dpi=300, bbox_inches='tight')
    print("Analysis plot saved: zigbee_comprehensive_analysis.png")
    
    # Print summary statistics
    print("\n" + "="*60)
    print("SUMMARY STATISTICS")
    print("="*60)
    print(f"Number of test cases: {len(df)}")
    print(f"Node range: {df['NumNodes'].min()} - {df['NumNodes'].max()}")
    print(f"\nAverage PDR: {df['PDR'].mean():.2f}% (±{df['PDR'].std():.2f}%)")
    print(f"Average Throughput: {df['ThroughputKbps'].mean():.2f} kbps (±{df['ThroughputKbps'].std():.2f})")
    print(f"Average Delay: {df['AvgDelay'].mean():.2f} ms (±{df['AvgDelay'].std():.2f})")
    print(f"Average SNR: {df['AvgSNR'].mean():.2f} dB (±{df['AvgSNR'].std():.2f})")
    print(f"\nTotal packets transmitted: {df['PacketsSent'].sum()}")
    print(f"Total packets received: {df['PacketsReceived'].sum()}")
    print(f"Total packets dropped by noise: {df['PacketsDroppedNoise'].sum()}")
    print(f"Total packets dropped by fading: {df['PacketsDroppedFading'].sum()}")
    print("="*60 + "\n")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_results.py <csv_file>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    analyze_zigbee_results(csv_file)
```

**Sử dụng:**
```bash
python3 analyze_results.py zigbee_performance.csv
```

---

## 🎓 Complete Research Workflow Example

```bash
#!/bin/bash
# complete_research_workflow.sh

PROJECT_NAME="zigbee_smart_home_study"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
WORK_DIR="${PROJECT_NAME}_${TIMESTAMP}"

echo "========================================"
echo "ZigBee Smart Home Research Workflow"
echo "========================================"
echo "Project: $PROJECT_NAME"
echo "Timestamp: $TIMESTAMP"
echo "Working Directory: $WORK_DIR"
echo "========================================"

# Create directory structure
mkdir -p $WORK_DIR/{logs,results,plots,reports}

# Phase 1: Baseline Test
echo -e "\n[Phase 1] Running baseline test..."
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --simTime=300 \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/baseline.csv" \
  > $WORK_DIR/logs/baseline.log 2>&1

# Phase 2: Scalability Test
echo -e "\n[Phase 2] Running scalability tests..."
for nodes in 3 5 8 10 12 15 18 20; do
  echo "  Testing with $nodes nodes..."
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --numNodes=$nodes \
    --simTime=300 \
    --exportCSV=true \
    --csvFile=$WORK_DIR/results/scalability.csv" \
    > $WORK_DIR/logs/scalability_${nodes}nodes.log 2>&1
  sleep 3
done

# Phase 3: Channel Quality Tests
echo -e "\n[Phase 3] Running channel quality tests..."

# No noise/fading
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=false \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/channel_ideal.csv" \
  > $WORK_DIR/logs/channel_ideal.log 2>&1

# Only noise
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=true \
  --enableFading=false \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/channel_noise.csv" \
  > $WORK_DIR/logs/channel_noise.log 2>&1

# Only fading
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=false \
  --enableFading=true \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/channel_fading.csv" \
  > $WORK_DIR/logs/channel_fading.log 2>&1

# Both noise and fading
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=10 \
  --enableNoise=true \
  --enableFading=true \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/channel_realistic.csv" \
  > $WORK_DIR/logs/channel_realistic.log 2>&1

# Phase 4: SNR Threshold Tests
echo -e "\n[Phase 4] Running SNR threshold tests..."
for snr in 3.0 6.0 9.0 12.0; do
  echo "  Testing SNR threshold: $snr dB..."
  ./ns3 run "smart-home-zigbee-complete-ver3 \
    --numNodes=10 \
    --snrThreshold=$snr \
    --exportCSV=true \
    --csvFile=$WORK_DIR/results/snr_threshold.csv" \
    > $WORK_DIR/logs/snr_${snr}dB.log 2>&1
  sleep 3
done

# Phase 5: Routing Comparison
echo -e "\n[Phase 5] Running routing comparison..."

# Many-to-One
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=15 \
  --manyToOne=true \
  --simTime=600 \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/routing_mto.csv" \
  > $WORK_DIR/logs/routing_mto.log 2>&1

# Mesh
./ns3 run "smart-home-zigbee-complete-ver3 \
  --numNodes=15 \
  --manyToOne=false \
  --simTime=600 \
  --exportCSV=true \
  --csvFile=$WORK_DIR/results/routing_mesh.csv" \
  > $WORK_DIR/logs/routing_mesh.log 2>&1

# Phase 6: Generate Analysis
echo -e "\n[Phase 6] Generating analysis and plots..."

# Analyze each result set
python3 analyze_results.py $WORK_DIR/results/scalability.csv
mv zigbee_comprehensive_analysis.png $WORK_DIR/plots/scalability_analysis.png

# Generate summary report
cat > $WORK_DIR/reports/summary.txt << EOF
ZigBee Smart Home Network Simulation - Research Summary
========================================================
Project: $PROJECT_NAME
Date: $TIMESTAMP

Test Phases Completed:
1. Baseline Test
2. Scalability Analysis (3-20 nodes)
3. Channel Quality Impact (4 scenarios)
4. SNR Threshold Sensitivity (4 levels)
5. Routing Protocol Comparison (Mesh vs Many-to-One)

Results Location:
- Raw Data: $WORK_DIR/results/
- Logs: $WORK_DIR/logs/
- Plots: $WORK_DIR/plots/
- Reports: $WORK_DIR/reports/

Key Files:
- scalability.csv: Network scalability data
- channel_*.csv: Channel quality impact data
- snr_threshold.csv: SNR sensitivity data
- routing_*.csv: Routing comparison data

Next Steps:
1. Review CSV files for detailed metrics
2. Analyze plots in plots/ directory
3. Compare results across different scenarios
4. Document findings and conclusions

========================================================
EOF

echo -e "\n========================================"
echo "Research workflow completed!"
echo "========================================"
echo "Results directory: $WORK_DIR"
echo "Summary report: $WORK_DIR/reports/summary.txt"
echo "========================================"
echo ""
echo "To view results:"
echo "  cat $WORK_DIR/reports/summary.txt"
echo "  ls -lh $WORK_DIR/results/"
echo "  ls -lh $WORK_DIR/plots/"
echo "========================================"
```

---

## 📝 Sample Output Interpretation

### Ví dụ output tốt:
```
PERFORMANCE METRICS
========================================
Network Size: 10 nodes
Simulation Time: 300s

--- Delivery Performance ---
  Packet Delivery Ratio: 96.50%    ✅ EXCELLENT
  Packet Loss Rate:      3.50%

--- Channel Quality Statistics ---
  Average SNR:           14.25 dB   ✅ GOOD
  Packets Dropped by Noise:  2
  Packets Dropped by Fading: 3

--- Throughput ---
  Throughput:            12.45 kbps ✅ GOOD

--- End-to-End Delay ---
  Average Delay:         45.2 ms    ✅ EXCELLENT
```

### Ví dụ output cần cải thiện:
```
PERFORMANCE METRICS
========================================
Network Size: 20 nodes
Simulation Time: 300s

--- Delivery Performance ---
  Packet Delivery Ratio: 78.30%    ⚠️ POOR
  Packet Loss Rate:      21.70%

--- Channel Quality Statistics ---
  Average SNR:           4.12 dB    ⚠️ BELOW THRESHOLD
  Packets Dropped by Noise:  15
  Packets Dropped by Fading: 18

--- Throughput ---
  Throughput:            3.21 kbps  ⚠️ LOW

--- End-to-End Delay ---
  Average Delay:         567.8 ms   ⚠️ HIGH
```

**Hành động cần thiết:**
- Giảm số nodes hoặc cải thiện topology
- Kiểm tra SNR threshold (có thể quá cao)
- Xem xét sử dụng Many-to-One routing
- Tăng công suất phát (sửa trong code)

---

## 🏁 Conclusion

Bạn đã có đầy đủ hướng dẫn để:
✅ Chạy mô phỏng với các cấu hình khác nhau
✅ Phân tích kết quả chi tiết
✅ Xuất và xử lý dữ liệu
✅ Trực quan hóa mạng
✅ Thực hiện nghiên cứu hoàn chỉnh

**Good luck with your ZigBee simulation research!** 🎉
