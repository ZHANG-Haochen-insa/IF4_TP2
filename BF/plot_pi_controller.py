"""
PI Controller Speed Control Data Visualization
"""

import matplotlib.pyplot as plt
import numpy as np

# Data
data = """4700	0.000	0.000	0.0
4800	0.000	0.000	0.0
4900	0.000	0.000	0.0
5000	2.500	0.000	17000.0
5100	2.500	2.808	-97.1
5200	2.500	1.856	6129.8
5300	2.500	1.285	10528.8
5400	2.500	2.332	4379.7
5500	2.500	1.952	7103.2
5600	2.500	1.761	8836.7
5700	2.500	2.237	6190.9
5800	2.500	2.047	7695.9
5900	2.500	2.094	7734.8
6000	2.500	2.142	7735.6
6100	2.500	2.190	7698.3
6200	2.500	2.237	7623.0
6300	2.500	2.190	8156.9
6400	2.500	2.237	8081.5
6500	2.500	2.285	7968.1
6600	2.500	2.285	8140.3
6700	2.500	2.285	8312.4
6800	2.500	2.332	8160.9
6900	2.500	2.332	8295.0
7000	2.500	2.332	8429.1
7100	2.500	2.380	8239.5
7200	2.500	2.380	8335.5
7300	2.500	2.332	8755.2
7400	2.500	2.428	8241.9
7500	2.500	2.380	8623.5
7600	2.500	2.428	8395.8
7700	2.500	2.380	8777.4
7800	2.500	2.428	8549.7
7900	2.500	2.428	8607.7
8000	2.500	2.428	8665.6
8100	2.500	2.475	8399.8
8200	2.500	2.380	9067.0
8300	2.500	2.475	8515.7
8400	2.500	2.475	8535.5
8500	2.500	2.428	8879.1
8600	2.500	2.475	8613.3
8700	2.500	2.475	8633.2
8800	2.500	2.475	8653.0
8900	2.500	2.428	8996.5
9000	2.500	2.523	8407.1
9100	2.500	2.428	9036.2
9200	2.500	2.523	8446.8
9300	2.500	2.428	9075.9
9400	2.500	2.475	8810.1
9500	2.500	2.523	8506.3
9600	2.500	2.428	9135.4
9700	2.500	2.523	8546.0
9800	2.500	2.475	8851.4
9900	2.500	2.475	8871.3
10000	-2.500	2.523	-34155.0
10100	-2.500	-5.188	14262.8
10200	-2.500	0.476	-22104.3
10300	-2.500	-3.142	114.5
10400	-2.500	-2.142	-6169.5
10500	-2.500	-1.333	-11958.4
10600	-2.500	-2.618	-4152.9
10700	-2.500	-2.094	-7618.9
10800	-2.500	-1.904	-9238.1
10900	-2.500	-2.380	-6478.1
11000	-2.500	-2.190	-7868.9
11100	-2.500	-2.142	-8440.9
11200	-2.500	-2.332	-7432.6
11300	-2.500	-2.285	-7890.3
11400	-2.500	-2.237	-8386.2
11500	-2.500	-2.332	-7949.1
11600	-2.500	-2.332	-8083.1
11700	-2.500	-2.332	-8217.2
11800	-2.500	-2.332	-8351.3
11900	-2.500	-2.380	-8161.7
12000	-2.500	-2.380	-8257.7
12100	-2.500	-2.332	-8677.4
12200	-2.500	-2.428	-8164.1
12300	-2.500	-2.428	-8222.0
12400	-2.500	-2.332	-8927.3
12500	-2.500	-2.475	-8090.4
12600	-2.500	-2.380	-8757.6
12700	-2.500	-2.475	-8206.2
12800	-2.500	-2.380	-8873.4
12900	-2.500	-2.475	-8322.1
13000	-2.500	-2.428	-8665.6
13100	-2.500	-2.475	-8399.8
13200	-2.500	-2.428	-8743.4
13300	-2.500	-2.428	-8801.3
13400	-2.500	-2.475	-8535.5
13500	-2.500	-2.475	-8555.4
13600	-2.500	-2.475	-8575.2
13700	-2.500	-2.428	-8918.8
13800	-2.500	-2.475	-8653.0
13900	-2.500	-2.523	-8349.2
14000	-2.500	-2.428	-8978.3
14100	-2.500	-2.475	-8712.5
14200	-2.500	-2.523	-8408.7
14300	-2.500	-2.428	-9037.8
14400	-2.500	-2.475	-8772.1
14500	-2.500	-2.523	-8468.2
14600	-2.500	-2.475	-8773.7
14700	-2.500	-2.475	-8793.5
14800	-2.500	-2.475	-8813.4
14900	-2.500	-2.523	-8509.5
15000	0.000	-2.475	16831.3
15100	0.000	2.285	-13556.4
15200	0.000	-1.238	8568.0
15300	0.000	0.333	-1123.4
15400	0.000	0.809	-4626.7
15500	0.000	-0.333	2494.2
15600	0.000	0.000	495.0
15700	0.000	0.048	171.4
15800	0.000	0.000	457.0
15900	0.000	0.000	457.0
16000	0.000	0.000	457.0
16100	0.000	0.000	457.0
16200	0.000	0.000	457.0
16300	0.000	0.000	457.0
16400	0.000	0.000	457.0
16500	0.000	0.000	457.0
16600	0.000	0.000	457.0
16700	0.000	0.000	457.0
16800	0.000	0.000	457.0"""

# Parse data
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

# Convert to numpy arrays
time_ms = np.array(time_ms)
time_s = time_ms / 1000  # Convert to seconds
setpoint = np.array(setpoint)
measured = np.array(measured)
control = np.array(control)

# Create figure
fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Top plot: Speed setpoint and measured
ax1 = axes[0]
ax1.plot(time_s, setpoint, 'b-', linewidth=2, label='Setpoint')
ax1.plot(time_s, measured, 'r-', linewidth=1.5, label='Measured')
ax1.set_ylabel('Angular Velocity (rad/s)', fontsize=12)
ax1.set_title('PI Controller - Left Wheel Speed Control (Kp=6000, Ki=8000)', fontsize=14)
ax1.legend(loc='upper right')
ax1.grid(True, alpha=0.3)
ax1.axhline(y=0, color='k', linestyle='-', linewidth=0.5)

# Bottom plot: Control signal
ax2 = axes[1]
ax2.plot(time_s, control, 'g-', linewidth=1.5, label='Control Signal')
ax2.set_xlabel('Time (s)', fontsize=12)
ax2.set_ylabel('PWM Control Signal', fontsize=12)
ax2.legend(loc='upper right')
ax2.grid(True, alpha=0.3)
ax2.axhline(y=0, color='k', linestyle='-', linewidth=0.5)

plt.tight_layout()
plt.savefig('pi_controller_response.png', dpi=150)
plt.show()

# Print steady-state error analysis
print("\n=== PI Controller Performance Analysis ===")

# Positive step (2.5 rad/s) steady-state analysis
pos_indices = (time_ms >= 9000) & (time_ms < 10000)
pos_steady_state = np.mean(measured[pos_indices])
pos_error = 2.5 - pos_steady_state
print(f"Positive step (2.5 rad/s):")
print(f"  Steady-state value: {pos_steady_state:.3f} rad/s")
print(f"  Steady-state error: {pos_error:.3f} rad/s ({abs(pos_error/2.5)*100:.1f}%)")

# Negative step (-2.5 rad/s) steady-state analysis
neg_indices = (time_ms >= 14000) & (time_ms < 15000)
neg_steady_state = np.mean(measured[neg_indices])
neg_error = -2.5 - neg_steady_state
print(f"Negative step (-2.5 rad/s):")
print(f"  Steady-state value: {neg_steady_state:.3f} rad/s")
print(f"  Steady-state error: {neg_error:.3f} rad/s ({abs(neg_error/2.5)*100:.1f}%)")

# Rise time analysis (10% to 90%)
print(f"\nRise time analysis:")
# For positive step
pos_start_idx = np.where(time_ms == 5000)[0][0]
target_10 = 0.1 * 2.5
target_90 = 0.9 * 2.5
pos_data = measured[pos_start_idx:pos_start_idx+50]
pos_time = time_ms[pos_start_idx:pos_start_idx+50]
try:
    t_10 = pos_time[np.where(pos_data >= target_10)[0][0]]
    t_90 = pos_time[np.where(pos_data >= target_90)[0][0]]
    print(f"  Positive step rise time (10%-90%): {t_90 - t_10:.0f} ms")
except:
    print(f"  Positive step rise time: Could not calculate")
