import cv2
import numpy as np
import glob
import imageio.v3 as iio

# User parameters
target_mp4_file = 'Output.mp4'
target_gif_file = 'Output.gif'
gif_deadtime = 100  # Duration in milliseconds
scaling_factor = 0.5

# Get list of images
files = sorted(glob.glob('*.png'))
if len(files) == 0:
    raise ValueError('No images found.')

# Initialize VideoWriter
h, w = cv2.imread(files[0], cv2.IMREAD_GRAYSCALE).shape
target_size = (int(w * scaling_factor), int(h * scaling_factor))
vid = cv2.VideoWriter(target_mp4_file, cv2.VideoWriter_fourcc(*'mp4v'), 10.0, target_size, isColor=False)

gif_frames = []

# Processing loop
for file in files:
    img = cv2.imread(file, cv2.IMREAD_GRAYSCALE)
    
    # Resize for video/GIF
    frame_s = cv2.resize(img, target_size, interpolation=cv2.INTER_NEAREST)
    
    # Add to video and GIF list
    vid.write(frame_s)
    gif_frames.append(frame_s)

# Finalize outputs
vid.release()
iio.imwrite(target_gif_file, gif_frames, duration=gif_deadtime, loop=0)

print('Conversion complete. MP4 and GIF saved.')