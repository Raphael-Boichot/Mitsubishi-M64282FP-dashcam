import cv2
import numpy as np
import os
import glob
import imageio.v3 as iio

# User parameters
target_mp4_file = 'Output.mp4'
target_gif_file = 'Output.gif'
gif_deadtime = 100  # imageio uses milliseconds (0.1s = 100ms)
scaling_factor = 0.5
v_border = 128
h_border = 128

# Create folder for full-size frames
output_folder = 'RGB_frames'
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

# Get list of images - verify these are in the same folder as the script
files = sorted(glob.glob('*.png'))
if len(files) < 3:
    raise ValueError(f'Not enough images found. Found {len(files)}, need at least 3.')

size_list = len(files) - (len(files) % 3)

# --- PASS 1: Calculate Weights ---
print('Analyzing frames...')
sum_R, sum_G, sum_B = 0.0, 0.0, 0.0
count = 0
for i in range(0, size_list, 3):
    sum_R += np.mean(cv2.imread(files[i], cv2.IMREAD_GRAYSCALE))
    sum_G += np.mean(cv2.imread(files[i+1], cv2.IMREAD_GRAYSCALE))
    sum_B += np.mean(cv2.imread(files[i+2], cv2.IMREAD_GRAYSCALE))
    count += 1

avg_R, avg_G, avg_B = sum_R/count, sum_G/count, sum_B/count
max_avg = max([avg_R, avg_G, avg_B])
weights = [max_avg/avg_R, max_avg/avg_G, max_avg/avg_B]

# --- PASS 2: Processing ---
h, w = cv2.imread(files[0], cv2.IMREAD_GRAYSCALE).shape
target_size = (int(w * scaling_factor), int(h * scaling_factor))
vid = cv2.VideoWriter(target_mp4_file, cv2.VideoWriter_fourcc(*'mp4v'), 10.0, target_size)

gif_frames = []

for i in range(0, size_list, 3):
    # Read raw
    R_raw = cv2.imread(files[i], cv2.IMREAD_GRAYSCALE).astype(np.float32)
    G_raw = cv2.imread(files[i+1], cv2.IMREAD_GRAYSCALE).astype(np.float32)
    B_raw = cv2.imread(files[i+2], cv2.IMREAD_GRAYSCALE).astype(np.float32)
    
    # White Balance
    R = np.clip(R_raw * weights[0], 0, 255)
    G = np.clip(G_raw * weights[1], 0, 255)
    B = np.clip(B_raw * weights[2], 0, 255)
    
    # Restore borders
    for channel, raw in [(R, R_raw), (G, G_raw), (B, B_raw)]:
        channel[:v_border, :] = raw[:v_border, :]
        channel[-v_border:, :] = raw[-v_border:, :]
        channel[:, :h_border] = raw[:, :h_border]
        channel[:, -h_border:] = raw[:, -h_border:]
    
    # Save full size frame
    full_frame = cv2.merge([B, G, R]).astype(np.uint8)
    cv2.imwrite(os.path.join(output_folder, f'frame_{i//3:04d}.png'), full_frame)
    
    # Prepare for Video/GIF (Resized)
    frame_s = cv2.resize(full_frame, target_size, interpolation=cv2.INTER_NEAREST)
    vid.write(frame_s)
    gif_frames.append(cv2.cvtColor(frame_s, cv2.COLOR_BGR2RGB))

# Finalize outputs
vid.release()
iio.imwrite(target_gif_file, gif_frames, duration=gif_deadtime, loop=0)
print('Conversion complete. MP4 and GIF saved.')