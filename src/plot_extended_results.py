#!/usr/bin/env python3
"""
Script vẽ biểu đồ mở rộng - Phân tích ảnh hưởng của:
- Khoảng cách giữa các node
- Số lượng node
- Nhiễu Gaussian và Rayleigh Fading
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import sys
import os

# Kiểm tra file tồn tại
csv_file = 'results_extended/zigbee_extended_results.csv'
if not os.path.exists(csv_file):
    print(f"❌ Không tìm thấy file: {csv_file}")
    print("Hãy chạy script bash trước để tạo dữ liệu!")
    sys.exit(1)

# Đọc dữ liệu
df = pd.read_csv(csv_file)

# Chuyển đổi boolean thành text dễ đọc
df['NoiseLabel'] = df['Noise'].map({1: 'Có Nhiễu', 0: 'Không Nhiễu'})
df['FadingLabel'] = df['Fading'].map({1: 'Có Fading', 0: 'Không Fading'})

# Lấy giá trị thực tế từ dữ liệu
available_distances = sorted(df['Distance'].unique())
available_nodes = sorted(df['NumNodes'].unique())

# Chọn giá trị trung bình để phân tích
mid_distance = available_distances[len(available_distances)//2] if len(available_distances) > 0 else 10
mid_nodes = available_nodes[len(available_nodes)//2] if len(available_nodes) > 0 else 6

print("=== Tổng quan dữ liệu ===")
print(f"Tổng số kịch bản: {len(df)}")
print(f"Khoảng cách có sẵn: {available_distances}")
print(f"Số node có sẵn: {available_nodes}")
print(f"Sử dụng distance={mid_distance}m và nodes={mid_nodes} cho phân tích")
print(f"\nMẫu dữ liệu:")
print(df.head())
print()

# Thiết lập style
plt.style.use('seaborn-v0_8-whitegrid')
sns.set_palette("husl")

# ============================================================
# Figure 1: Ảnh hưởng của KHOẢNG CÁCH
# ============================================================
fig1, axes1 = plt.subplots(2, 2, figsize=(16, 12))
fig1.suptitle('Ảnh hưởng của KHOẢNG CÁCH lên hiệu năng mạng ZigBee', 
              fontsize=16, fontweight='bold')

# 1.1: PDR vs Distance (với số node cố định)
ax1 = axes1[0, 0]
df_fixed_nodes = df[df['NumNodes'] == mid_nodes]

for (noise, fading), group in df_fixed_nodes.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax1.plot(group['Distance'], group['PDR'], marker='o', linewidth=2, 
             markersize=8, label=label)

ax1.set_xlabel('Khoảng cách giữa các node (m)', fontsize=11)
ax1.set_ylabel('PDR (%)', fontsize=11)
ax1.set_title(f'PDR vs Khoảng cách ({mid_nodes} nodes)', fontsize=12, fontweight='bold')
ax1.legend(loc='best')
ax1.grid(True, alpha=0.3)

# 1.2: SNR vs Distance
ax2 = axes1[0, 1]
for (noise, fading), group in df_fixed_nodes.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax2.plot(group['Distance'], group['AvgSNR'], marker='s', linewidth=2, 
             markersize=8, label=label)

ax2.set_xlabel('Khoảng cách giữa các node (m)', fontsize=11)
ax2.set_ylabel('SNR trung bình (dB)', fontsize=11)
ax2.set_title(f'SNR vs Khoảng cách ({mid_nodes} nodes)', fontsize=12, fontweight='bold')
ax2.axhline(y=4, color='red', linestyle='--', alpha=0.5, label='Ngưỡng SNR')
ax2.legend(loc='best')
ax2.grid(True, alpha=0.3)

# 1.3: Delay vs Distance
ax3 = axes1[1, 0]
for (noise, fading), group in df_fixed_nodes.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax3.plot(group['Distance'], group['AvgDelay'], marker='^', linewidth=2, 
             markersize=8, label=label)

ax3.set_xlabel('Khoảng cách giữa các node (m)', fontsize=11)
ax3.set_ylabel('Delay trung bình (ms)', fontsize=11)
ax3.set_title(f'Delay vs Khoảng cách ({mid_nodes} nodes)', fontsize=12, fontweight='bold')
ax3.legend(loc='best')
ax3.grid(True, alpha=0.3)

# 1.4: Heatmap PDR vs Distance (tất cả trường hợp)
ax4 = axes1[1, 1]
pivot_distance = df_fixed_nodes.pivot_table(
    values='PDR', 
    index=['Noise', 'Fading'], 
    columns='Distance'
)
sns.heatmap(pivot_distance, annot=True, fmt='.1f', cmap='RdYlGn', 
            ax=ax4, cbar_kws={'label': 'PDR (%)'}, vmin=0, vmax=100)
ax4.set_title('Heatmap: PDR theo Khoảng cách và Điều kiện kênh', 
              fontsize=12, fontweight='bold')
ax4.set_xlabel('Khoảng cách (m)', fontsize=11)
ax4.set_ylabel('Điều kiện kênh', fontsize=11)

plt.tight_layout()
plt.savefig('results_extended/analysis_distance.png', dpi=150, bbox_inches='tight')
print("✓ Đã lưu: analysis_distance.png")

# ============================================================
# Figure 2: Ảnh hưởng của SỐ LƯỢNG NODE
# ============================================================
fig2, axes2 = plt.subplots(2, 2, figsize=(16, 12))
fig2.suptitle('Ảnh hưởng của SỐ LƯỢNG NODE lên hiệu năng mạng ZigBee', 
              fontsize=16, fontweight='bold')

# 2.1: PDR vs Number of Nodes (với khoảng cách cố định)
ax1 = axes2[0, 0]
df_fixed_dist = df[df['Distance'] == mid_distance]

for (noise, fading), group in df_fixed_dist.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax1.plot(group['NumNodes'], group['PDR'], marker='o', linewidth=2, 
             markersize=8, label=label)

ax1.set_xlabel('Số lượng node', fontsize=11)
ax1.set_ylabel('PDR (%)', fontsize=11)
ax1.set_title(f'PDR vs Số node (Distance = {mid_distance}m)', fontsize=12, fontweight='bold')
ax1.set_xticks(available_nodes)
ax1.legend(loc='best')
ax1.grid(True, alpha=0.3)

# 2.2: SNR vs Number of Nodes
ax2 = axes2[0, 1]
for (noise, fading), group in df_fixed_dist.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax2.plot(group['NumNodes'], group['AvgSNR'], marker='s', linewidth=2, 
             markersize=8, label=label)

ax2.set_xlabel('Số lượng node', fontsize=11)
ax2.set_ylabel('SNR trung bình (dB)', fontsize=11)
ax2.set_title(f'SNR vs Số node (Distance = {mid_distance}m)', fontsize=12, fontweight='bold')
ax2.set_xticks(available_nodes)
ax2.axhline(y=4, color='red', linestyle='--', alpha=0.5, label='Ngưỡng SNR')
ax2.legend(loc='best')
ax2.grid(True, alpha=0.3)

# 2.3: Delay vs Number of Nodes
ax3 = axes2[1, 0]
for (noise, fading), group in df_fixed_dist.groupby(['Noise', 'Fading']):
    label = f"Noise={noise}, Fading={fading}"
    ax3.plot(group['NumNodes'], group['AvgDelay'], marker='^', linewidth=2, 
             markersize=8, label=label)

ax3.set_xlabel('Số lượng node', fontsize=11)
ax3.set_ylabel('Delay trung bình (ms)', fontsize=11)
ax3.set_title(f'Delay vs Số node (Distance = {mid_distance}m)', fontsize=12, fontweight='bold')
ax3.set_xticks(available_nodes)
ax3.legend(loc='best')
ax3.grid(True, alpha=0.3)

# 2.4: Heatmap PDR vs Number of Nodes
ax4 = axes2[1, 1]
pivot_nodes = df_fixed_dist.pivot_table(
    values='PDR', 
    index=['Noise', 'Fading'], 
    columns='NumNodes'
)
sns.heatmap(pivot_nodes, annot=True, fmt='.1f', cmap='RdYlGn', 
            ax=ax4, cbar_kws={'label': 'PDR (%)'}, vmin=0, vmax=100)
ax4.set_title('Heatmap: PDR theo Số node và Điều kiện kênh', 
              fontsize=12, fontweight='bold')
ax4.set_xlabel('Số lượng node', fontsize=11)
ax4.set_ylabel('Điều kiện kênh', fontsize=11)

plt.tight_layout()
plt.savefig('results_extended/analysis_num_nodes.png', dpi=150, bbox_inches='tight')
print("✓ Đã lưu: analysis_num_nodes.png")

# ============================================================
# Figure 3: Ma trận tổng hợp Distance x NumNodes
# ============================================================
fig3, axes3 = plt.subplots(2, 2, figsize=(16, 12))
fig3.suptitle('Ma trận tổng hợp: Khoảng cách × Số node', 
              fontsize=16, fontweight='bold')

conditions = [
    (0, 0, 'Ideal (No Noise, No Fading)'),
    (1, 0, 'Noise Only'),
    (0, 1, 'Fading Only'),
    (1, 1, 'Realistic (Noise + Fading)')
]

for idx, (noise, fading, title) in enumerate(conditions):
    ax = axes3[idx // 2, idx % 2]
    
    df_condition = df[(df['Noise'] == noise) & (df['Fading'] == fading)]
    
    if len(df_condition) > 0:
        pivot = df_condition.pivot_table(
            values='PDR',
            index='Distance',
            columns='NumNodes'
        )
        
        sns.heatmap(pivot, annot=True, fmt='.1f', cmap='RdYlGn', 
                    ax=ax, cbar_kws={'label': 'PDR (%)'}, vmin=0, vmax=100)
        ax.set_title(f'PDR - {title}', fontsize=12, fontweight='bold')
        ax.set_xlabel('Số node', fontsize=11)
        ax.set_ylabel('Khoảng cách (m)', fontsize=11)
    else:
        ax.text(0.5, 0.5, 'Không có dữ liệu', ha='center', va='center', transform=ax.transAxes)

plt.tight_layout()
plt.savefig('results_extended/analysis_matrix.png', dpi=150, bbox_inches='tight')
print("✓ Đã lưu: analysis_matrix.png")

# ============================================================
# Figure 4: So sánh ảnh hưởng của từng yếu tố
# ============================================================
fig4, axes4 = plt.subplots(2, 2, figsize=(16, 12))
fig4.suptitle('Phân tích tác động của từng yếu tố', 
              fontsize=16, fontweight='bold')

# 4.1: Ảnh hưởng riêng của Noise
ax1 = axes4[0, 0]
no_fading = df[df['Fading'] == 0]
for distance in available_distances:
    df_dist = no_fading[no_fading['Distance'] == distance]
    df_no_noise = df_dist[df_dist['Noise'] == 0].sort_values('NumNodes')
    df_with_noise = df_dist[df_dist['Noise'] == 1].sort_values('NumNodes')
    
    if len(df_no_noise) > 0 and len(df_with_noise) > 0:
        # Đảm bảo cùng index NumNodes
        merged = pd.merge(df_no_noise[['NumNodes', 'PDR']], 
                         df_with_noise[['NumNodes', 'PDR']], 
                         on='NumNodes', suffixes=('_off', '_on'))
        impact = merged['PDR_off'] - merged['PDR_on']
        ax1.plot(merged['NumNodes'], impact, marker='o', label=f'{distance}m', linewidth=2)

ax1.set_xlabel('Số lượng node', fontsize=11)
ax1.set_ylabel('Giảm PDR do Nhiễu (%)', fontsize=11)
ax1.set_title('Tác động của Nhiễu Gaussian (không có Fading)', 
              fontsize=12, fontweight='bold')
ax1.legend(title='Khoảng cách')
ax1.grid(True, alpha=0.3)
ax1.axhline(y=0, color='black', linestyle='-', linewidth=0.5)

# 4.2: Ảnh hưởng riêng của Fading
ax2 = axes4[0, 1]
no_noise = df[df['Noise'] == 0]
for distance in available_distances:
    df_dist = no_noise[no_noise['Distance'] == distance]
    df_no_fading = df_dist[df_dist['Fading'] == 0].sort_values('NumNodes')
    df_with_fading = df_dist[df_dist['Fading'] == 1].sort_values('NumNodes')
    
    if len(df_no_fading) > 0 and len(df_with_fading) > 0:
        merged = pd.merge(df_no_fading[['NumNodes', 'PDR']], 
                         df_with_fading[['NumNodes', 'PDR']], 
                         on='NumNodes', suffixes=('_off', '_on'))
        impact = merged['PDR_off'] - merged['PDR_on']
        ax2.plot(merged['NumNodes'], impact, marker='s', label=f'{distance}m', linewidth=2)

ax2.set_xlabel('Số lượng node', fontsize=11)
ax2.set_ylabel('Giảm PDR do Fading (%)', fontsize=11)
ax2.set_title('Tác động của Rayleigh Fading (không có Nhiễu)', 
              fontsize=12, fontweight='bold')
ax2.legend(title='Khoảng cách')
ax2.grid(True, alpha=0.3)
ax2.axhline(y=0, color='black', linestyle='-', linewidth=0.5)

# 4.3: Tác động kết hợp
ax3 = axes4[1, 0]
for distance in available_distances:
    df_dist = df[df['Distance'] == distance]
    df_ideal = df_dist[(df_dist['Noise'] == 0) & (df_dist['Fading'] == 0)].sort_values('NumNodes')
    df_realistic = df_dist[(df_dist['Noise'] == 1) & (df_dist['Fading'] == 1)].sort_values('NumNodes')
    
    if len(df_ideal) > 0 and len(df_realistic) > 0:
        merged = pd.merge(df_ideal[['NumNodes', 'PDR']], 
                         df_realistic[['NumNodes', 'PDR']], 
                         on='NumNodes', suffixes=('_ideal', '_real'))
        impact = merged['PDR_ideal'] - merged['PDR_real']
        ax3.plot(merged['NumNodes'], impact, marker='^', label=f'{distance}m', linewidth=2)

ax3.set_xlabel('Số lượng node', fontsize=11)
ax3.set_ylabel('Giảm PDR do Nhiễu + Fading (%)', fontsize=11)
ax3.set_title('Tác động kết hợp (Nhiễu + Fading)', 
              fontsize=12, fontweight='bold')
ax3.legend(title='Khoảng cách')
ax3.grid(True, alpha=0.3)
ax3.axhline(y=0, color='black', linestyle='-', linewidth=0.5)

# 4.4: Bar chart so sánh độ suy giảm
ax4 = axes4[1, 1]
conditions_names = ['Ideal', 'Noise Only', 'Fading Only', 'Realistic']
avg_pdr = []

for noise, fading, name in conditions:
    pdr = df[(df['Noise'] == noise) & (df['Fading'] == fading)]['PDR'].mean()
    avg_pdr.append(pdr)

colors_bar = ['#2ecc71', '#3498db', '#e74c3c', '#9b59b6']
bars = ax4.bar(conditions_names, avg_pdr, color=colors_bar, edgecolor='black', linewidth=1.5)
ax4.set_ylabel('PDR trung bình (%)', fontsize=11)
ax4.set_title('So sánh PDR trung bình theo điều kiện kênh', 
              fontsize=12, fontweight='bold')
ax4.set_ylim(0, 105)

for bar, val in zip(bars, avg_pdr):
    ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1, 
             f'{val:.1f}%', ha='center', va='bottom', fontsize=10, fontweight='bold')

plt.tight_layout()
plt.savefig('results_extended/analysis_impact.png', dpi=150, bbox_inches='tight')
print("✓ Đã lưu: analysis_impact.png")

plt.show()

# ============================================================
# Bảng tóm tắt thống kê
# ============================================================
print("\n" + "="*80)
print("BẢNG THỐNG KÊ TỔNG HỢP")
print("="*80)

summary_stats = df.groupby(['Distance', 'NumNodes', 'Noise', 'Fading']).agg({
    'PDR': 'mean',
    'AvgSNR': 'mean',
    'AvgDelay': 'mean'
}).round(2)

print(summary_stats.to_string())

print("\n" + "="*80)
print("KẾT LUẬN CHỦ YẾU")
print("="*80)

# Phân tích xu hướng với giá trị thực tế
print("\n1. Ảnh hưởng của KHOẢNG CÁCH:")
if len(available_distances) >= 2:
    dist_min = available_distances[0]
    dist_max = available_distances[-1]
    
    for noise, fading, name in conditions:
        df_cond = df[(df['Noise'] == noise) & (df['Fading'] == fading) & (df['NumNodes'] == mid_nodes)]
        if len(df_cond) > 1:
            pdr_min = df_cond[df_cond['Distance'] == dist_min]['PDR'].values
            pdr_max = df_cond[df_cond['Distance'] == dist_max]['PDR'].values
            if len(pdr_min) > 0 and len(pdr_max) > 0:
                print(f"   • {name}: PDR giảm {pdr_min[0] - pdr_max[0]:.1f}% khi tăng từ {dist_min}m → {dist_max}m")

print("\n2. Ảnh hưởng của SỐ LƯỢNG NODE:")
if len(available_nodes) >= 2:
    nodes_min = available_nodes[0]
    nodes_max = available_nodes[-1]
    
    for noise, fading, name in conditions:
        df_cond = df[(df['Noise'] == noise) & (df['Fading'] == fading) & (df['Distance'] == mid_distance)]
        if len(df_cond) > 1:
            pdr_min = df_cond[df_cond['NumNodes'] == nodes_min]['PDR'].values
            pdr_max = df_cond[df_cond['NumNodes'] == nodes_max]['PDR'].values
            if len(pdr_min) > 0 and len(pdr_max) > 0:
                change = pdr_max[0] - pdr_min[0]
                trend = "tăng" if change > 0 else "giảm"
                print(f"   • {name}: PDR {trend} {abs(change):.1f}% khi tăng từ {nodes_min} → {nodes_max} nodes")

print("\n" + "="*80)
