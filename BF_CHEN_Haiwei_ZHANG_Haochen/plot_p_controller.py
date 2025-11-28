"""
P Controller Speed Control Data Visualization
"""

import matplotlib.pyplot as plt
import numpy as np

# 数据
data = """600	0.000	0.000	0.0
700	0.000	0.000	0.0
800	0.000	0.000	0.0
900	0.000	0.000	0.0
1000	2.500	0.000	7500.0
1100	2.500	0.809	5072.4
1200	2.500	1.428	3216.0
1300	2.500	1.047	4358.4
1400	2.500	0.809	5072.4
1500	2.500	1.047	4358.4
1600	2.500	1.095	4215.6
1700	2.500	0.952	4644.0
1800	2.500	1.000	4501.2
1900	2.500	1.000	4501.2
2000	-2.500	1.047	-10641.6
2100	-2.500	-1.428	-3216.0
2200	-2.500	-1.666	-2502.0
2300	-2.500	-0.714	-5358.0
2400	-2.500	-0.809	-5072.4
2500	-2.500	-1.142	-4072.8
2600	-2.500	-1.047	-4358.4
2700	-2.500	-0.904	-4786.8
2800	-2.500	-0.952	-4644.0
2900	-2.500	-1.047	-4358.4
3000	0.000	-1.000	2998.8
3100	0.000	-0.190	571.2
3200	0.000	0.000	0.0
3300	0.000	0.000	0.0
3400	0.000	0.000	0.0
3500	0.000	0.000	0.0"""

# 解析数据
lines = data.strip().split('\n')
time_ms = []
setpoint = []
measured = []
control = []

for line in lines:
    parts = line.split('\t')
    time_ms.append(float(parts[0]))
    setpoint.append(float(parts[1]))
    measured.append(float(parts[2]))
    control.append(float(parts[3]))

# 转换为numpy数组
time_ms = np.array(time_ms)
time_s = time_ms / 1000  # 转换为秒
setpoint = np.array(setpoint)
measured = np.array(measured)
control = np.array(control)

# 创建图形
fig, axes = plt.subplots(2, 1, figsize=(10, 8), sharex=True)

# 上图: 速度设定点和测量值
ax1 = axes[0]
ax1.plot(time_s, setpoint, 'b-', linewidth=2, label='Setpoint')
ax1.plot(time_s, measured, 'r-', linewidth=1.5, label='Measured')
ax1.set_ylabel('Angular Velocity (rad/s)', fontsize=12)
ax1.set_title('P Controller - Left Wheel Speed Control', fontsize=14)
ax1.legend(loc='upper right')
ax1.grid(True, alpha=0.3)
ax1.axhline(y=0, color='k', linestyle='-', linewidth=0.5)

# 下图: 控制信号
ax2 = axes[1]
ax2.plot(time_s, control, 'g-', linewidth=1.5, label='Control Signal')
ax2.set_xlabel('Time (s)', fontsize=12)
ax2.set_ylabel('PWM Control Signal', fontsize=12)
ax2.legend(loc='upper right')
ax2.grid(True, alpha=0.3)
ax2.axhline(y=0, color='k', linestyle='-', linewidth=0.5)

plt.tight_layout()
plt.savefig('p_controller_response.png', dpi=150)
plt.show()

# Print steady-state error analysis
print("\n=== P Controller Performance Analysis ===")
# Positive step (2.5 rad/s) steady-state analysis
pos_indices = (time_ms >= 1500) & (time_ms < 2000)
pos_steady_state = np.mean(measured[pos_indices])
pos_error = 2.5 - pos_steady_state
print(f"Positive step (2.5 rad/s):")
print(f"  Steady-state value: {pos_steady_state:.3f} rad/s")
print(f"  Steady-state error: {pos_error:.3f} rad/s ({pos_error/2.5*100:.1f}%)")

# Negative step (-2.5 rad/s) steady-state analysis
neg_indices = (time_ms >= 2500) & (time_ms < 3000)
neg_steady_state = np.mean(measured[neg_indices])
neg_error = -2.5 - neg_steady_state
print(f"Negative step (-2.5 rad/s):")
print(f"  Steady-state value: {neg_steady_state:.3f} rad/s")
print(f"  Steady-state error: {neg_error:.3f} rad/s ({abs(neg_error/2.5)*100:.1f}%)")
