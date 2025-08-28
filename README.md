# Seam Carving


### Approach

The program implements the content-aware image resizing algorithm known as Seam Carving.

1.  **Energy Calculation**: The energy of each pixel is calculated using a **dual-gradient energy function**. This function computes the sum of the squared color differences between the pixel's horizontal neighbors (left and right) and its vertical neighbors (up and down).

2.  **Seam Identification**: A **dynamic programming** approach is used to find the lowest-energy seam. A cumulative energy matrix is built from top to bottom, where each cell `(i, j)` stores the minimum possible energy of a seam ending at that pixel. The lowest-energy seam is then found by backtracking from the minimum value in the last row of this matrix.

3.  **Seam Removal**:
    * **Width Reduction**: To reduce the width, the program iteratively finds and removes the lowest-energy vertical seam.
    * **Height Reduction**: To reduce the height, the image is first **rotated by 90 degrees counter-clockwise**. The same vertical seam carving logic is then applied, which effectively removes a horizontal seam from the original image perspective. Finally, the image is rotated back 90 degrees clockwise.

The process is repeated until the image is resized to the target dimensions.


**Commands to compile and execute:**
            
        $ g++ 2025201006_A1_Q1.cpp -o q1 $(pkg-config --cflags --libs opencv4)
        $ ./q1 ./input_path req_width req_height
