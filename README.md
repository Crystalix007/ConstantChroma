# ConstantChroma
Constant chroma for image sequences

To build on *nix: ./Build.sh

Needs OpenCV libraries installed.

May have perceptual colour rendering shifts.

## What does this do?
This tool takes a folder full of frames, and uses the first frame to obtain a constant brightness for other frames using a technique known as histogram matching.

To use it, customise the constants in the file, build the tool as above, then, from inside a folder of images (preferably named 'Frame00001.png', 'Frame00002.png', 'Frame00003.png', etc. with no missing numbers), run ```PATH/TO/SOLVER``` within the folder.
There should be a subfolder (inside the folder with the images) to store the edited images: by default, 'processed'.
