#!/bin/bash
# ============================================================
# Script mô phỏng ZigBee Indoor Smart Home
# Khảo sát: Khoảng cách + Số lượng node + Nhiễu + Fading
# 
# Tối ưu cho môi trường INDOOR SMART HOME:
# - Khoảng cách: 5m, 10m, 15m, 20m (khoảng cách điển hình trong nhà)
# - Số node: 4, 6, 8, 10 (quy mô mạng gia đình thông minh)
# - Path loss exponent: 3.0 (indoor với tường và đồ đạc)
# - TX Power: 0 dBm (1mW - chuẩn ZigBee)
# ============================================================

OUTPUT_DIR="results_extended"
mkdir -p $OUTPUT_DIR

echo "=========================================="
echo "ZigBee Indoor Smart Home Simulation"
echo "Khảo sát: Distance, Number of Nodes, Noise, Fading"
echo "=========================================="

# Tham số khảo sát - INDOOR SMART HOME
DISTANCES=(5 10 15 20)            # Khoảng cách trong nhà (m)
NUM_NODES=(4 6 8 10)               # Số lượng thiết bị trong nhà
NOISE_VALUES=(true false)          # Có/không nhiễu
FADING_VALUES=(true false)         # Có/không fading

TOTAL_RUNS=$((${#DISTANCES[@]} * ${#NUM_NODES[@]} * ${#NOISE_VALUES[@]} * ${#FADING_VALUES[@]}))
CURRENT_RUN=0

echo "Tổng số lần chạy: $TOTAL_RUNS"
echo ""
echo "Thông số mô phỏng Indoor Smart Home:"
echo "Khoảng cách: ${DISTANCES[@]} m (khoảng cách giữa các phòng)"
echo "Số thiết bị: ${NUM_NODES[@]} nodes"
echo "Path loss exp: 3.0 (indoor với vật cản)"
echo "TX Power: 0 dBm (1mW - điển hình cho ZigBee)"
echo ""

# Xóa file CSV cũ nếu có
CSV_OUTPUT="$OUTPUT_DIR/zigbee_extended_results.csv"
rm -f $CSV_OUTPUT

# Đếm số lần thất bại
FAILED_RUNS=0

# Vòng lặp qua tất cả các kịch bản
for distance in "${DISTANCES[@]}"; do
    for num_nodes in "${NUM_NODES[@]}"; do
        for noise in "${NOISE_VALUES[@]}"; do
            for fading in "${FADING_VALUES[@]}"; do
                CURRENT_RUN=$((CURRENT_RUN + 1))
                
                # Tạo tên kịch bản
                SCENARIO="D${distance}_N${num_nodes}"
                if [ "$noise" = "true" ]; then
                    SCENARIO="${SCENARIO}_Noise"
                fi
                if [ "$fading" = "true" ]; then
                    SCENARIO="${SCENARIO}_Fading"
                fi
                
                echo "=========================================="
                echo "[$CURRENT_RUN/$TOTAL_RUNS] Running: $SCENARIO"
                echo "  Distance: ${distance}m (indoor range)"
                echo "  Nodes: $num_nodes"
                echo "  Noise: $noise"
                echo "  Fading: $fading"
                echo "=========================================="
                
                # Chạy mô phỏng với path loss exponent = 3.0 (indoor)
                ./ns3 run "zigbee-extended-sim \
                    --distance=$distance \
                    --nodes=$num_nodes \
                    --noise=$noise \
                    --fading=$fading \
                    --scenario=$SCENARIO \
                    --csv=$CSV_OUTPUT \
                    --pathLossExp=3.0 \
                    --packets=50 \
                    --interval=2.0 \
                    --time=120" 2>&1 | tail -25
                
                # Kiểm tra kết quả
                if [ ${PIPESTATUS[0]} -ne 0 ]; then
                    echo "Lỗi khi chạy kịch bản $SCENARIO"
                    FAILED_RUNS=$((FAILED_RUNS + 1))
                else
                    echo "Hoàn thành $SCENARIO"
                fi
                
                echo ""
                sleep 1
            done
        done
    done
done

echo ""
echo "=========================================="
echo "✓ Hoàn thành tất cả $TOTAL_RUNS kịch bản!"
if [ $FAILED_RUNS -gt 0 ]; then
    echo "Số lần chạy thất bại: $FAILED_RUNS"
fi
echo "=========================================="
echo "Kết quả lưu tại: $CSV_OUTPUT"
echo ""

# Hiển thị thống kê tổng quan
if [ -f "$CSV_OUTPUT" ]; then
    echo "📈 Thống kê tổng quan:"
    echo "  Tổng số dòng dữ liệu: $(wc -l < $CSV_OUTPUT)"
    echo ""
    echo "🔍 5 dòng đầu tiên:"
    head -6 $CSV_OUTPUT | column -t -s,
    echo ""
    
    # Thống kê nhanh
    echo "📊 Thống kê nhanh PDR:"
    tail -n +2 $CSV_OUTPUT | awk -F',' '{sum+=$12; count++} END {printf "  • PDR trung bình: %.2f%%\n", sum/count}'
    tail -n +2 $CSV_OUTPUT | awk -F',' '{if ($12 > max) max=$12; if (NR==2 || $12 < min) min=$12} END {printf "  • PDR cao nhất: %.2f%%\n  • PDR thấp nhất: %.2f%%\n", max, min}'
    echo ""
fi

# Chạy script vẽ biểu đồ
echo "=========================================="
echo "📊 Đang tạo biểu đồ phân tích..."
echo "=========================================="

if [ -f "plot_extended_results.py" ]; then
    python3 plot_extended_results.py
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Biểu đồ đã được tạo thành công!"
        echo ""
        echo "📁 Các file biểu đồ:"
        ls -lh $OUTPUT_DIR/*.png 2>/dev/null | awk '{print "  •", $9, "(" $5 ")"}'
    else
        echo "❌ Có lỗi khi tạo biểu đồ"
    fi
else
    echo "⚠️  Không tìm thấy plot_extended_results.py"
    echo "Hãy chạy script Python riêng để tạo biểu đồ"
fi

echo ""
echo "=========================================="
echo "🎯 HƯỚNG DẪN TIẾP THEO:"
echo "=========================================="
echo "1️⃣  Xem kết quả CSV:"
echo "   cat $CSV_OUTPUT"
echo ""
echo "2️⃣  Vẽ biểu đồ (nếu chưa tự động):"
echo "   python3 plot_extended_results.py"
echo ""
echo "3️⃣  Xem visualization NetAnim:"
echo "   netanim zigbee-indoor.xml"
echo ""
echo "4️⃣  Phân tích dữ liệu:"
echo "   • File CSV: $CSV_OUTPUT"
echo "   • Biểu đồ khoảng cách: $OUTPUT_DIR/analysis_distance.png"
echo "   • Biểu đồ số node: $OUTPUT_DIR/analysis_num_nodes.png"
echo "   • Ma trận tổng hợp: $OUTPUT_DIR/analysis_matrix.png"
echo "   • Phân tích tác động: $OUTPUT_DIR/analysis_impact.png"
echo "=========================================="
echo ""
echo "💡 Tip: Nếu muốn chạy với tham số khác, sửa file:"
echo "   nano run_indoor_simulation.sh"
echo "=========================================="
