import cv2
import os

# Path to the frames folder
frames_folder = "frames"

# Output video file name
output_video = "output_video.mp4"

# Get a list of all PNG files in the frames folder and sort them by name
png_files = [f for f in os.listdir(frames_folder) if f.endswith(".png")]
png_files.sort(key=lambda x: int(x.split("_")[1].split(".")[0]))

# Get the first frame to get the dimensions of the video
first_frame = cv2.imread(os.path.join(frames_folder, png_files[0]))
height, width, layers = first_frame.shape

# Define the codec and create a VideoWriter object
fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # You can change the codec as needed
out = cv2.VideoWriter(output_video, fourcc, 30.0, (width, height))

# Iterate through the sorted PNG files and add them to the video
for png_file in png_files:
    frame = cv2.imread(os.path.join(frames_folder, png_file))
    out.write(frame)

# Release the VideoWriter
out.release()

print("Video created:", output_video)
