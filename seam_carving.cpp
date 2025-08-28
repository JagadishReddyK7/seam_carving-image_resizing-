#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void bgr(Mat& image, int x, int y, int* arr) {
    arr[0]=0;
    arr[1]=0;
    arr[2]=0;
    if(x>=0 && x<image.rows && y>=0 && y<image.cols) {
        Vec3b pix=image.at<Vec3b>(x,y);
        arr[0]=(int)pix[0];
        arr[1]=(int)pix[1];
        arr[2]=(int)pix[2];
    }
}

int calculate_energy_cell(Mat& image, int x, int y) {
    int* left=new int[3];
    int* right=new int[3];
    int* up=new int[3];
    int* down=new int[3];
    bgr(image, x,y-1, left);
    bgr(image, x,y+1, right);
    bgr(image, x-1,y,up);
    bgr(image, x+1,y,down);
    int energy=sqrt(pow(left[0]-right[0],2)+pow(left[1]-right[1],2)+pow(left[2]-right[2],2)+pow(up[0]-down[0],2)+pow(up[1]-down[1],2)+pow(up[2]-down[2],2));
    delete[] left;
    delete[] right;
    delete[] up;
    delete[] down;
    return energy;
}

void calculate_energy_mat(Mat& image, int** energy_mat) {
    for(int i=0;i<image.rows;i++) {
        for(int j=0;j<image.cols;j++) {
            energy_mat[i][j]=calculate_energy_cell(image, i, j);
        }
    }
}

int* calculate_seam(int** energy_mat, int r, int c) {
    for(int i=1;i<r;i++) {
        for(int j=0;j<c;j++) {
            if(j==0 && j+1<c) energy_mat[i][j]+=min(energy_mat[i-1][j], energy_mat[i-1][j+1]);
            else if(j==c-1) energy_mat[i][j]+=min(energy_mat[i-1][j-1],energy_mat[i-1][j]);
            else energy_mat[i][j]+=min({energy_mat[i-1][j-1],energy_mat[i-1][j], energy_mat[i-1][j+1]});
        }
    }
    int col_num;
    int min_energy=INT_MAX;
    for(int i=0;i<c;i++) {
        if(energy_mat[r-1][i]<min_energy) {
            min_energy=energy_mat[r-1][i];
            col_num=i;
        }
    }
    int* seam_col=new int[r];
    seam_col[r-1]=col_num;
    for(int i=r-1;i>0;i--) {
        min_energy=INT_MAX;
        int curr_col=col_num;
        if(curr_col!=0) {
            if(energy_mat[i-1][curr_col-1]<min_energy) {
                min_energy=energy_mat[i-1][curr_col-1];
                col_num=curr_col-1;
            }
        }
        if(curr_col<c-1) {
            if(energy_mat[i-1][curr_col+1]<min_energy) {
                min_energy=energy_mat[i-1][curr_col+1];
                col_num=curr_col+1;
            }
        }
        if(energy_mat[i-1][curr_col]<min_energy) {
            min_energy=energy_mat[i-1][curr_col];
            col_num=curr_col;
        }
        seam_col[i-1]=col_num;
    }
    return seam_col;
}

Mat carve_seam(Mat& image, int* seam) {
    int r=image.rows;
    int c=image.cols;
    Mat carved_image(r, c-1, image.type());
    for(int i=0;i<r;i++) {
        for(int j=0;j<seam[i];j++) {
            carved_image.at<Vec3b>(i,j)=image.at<Vec3b>(i,j);
        }
        for(int j=seam[i]+1;j<c;j++) {
            carved_image.at<Vec3b>(i,j-1)=image.at<Vec3b>(i,j); 
        }
    }
    return carved_image;
}

void left_rotate(Mat& image, Mat& rotated_image) {
    int r=image.rows;
    int c=image.cols;
    for(int i=0;i<r;i++) {
        for(int j=0;j<c;j++) {
            rotated_image.at<Vec3b>(c-j-1,i)=image.at<Vec3b>(i,j);
        }
    }
}

void right_rotate(Mat& image, Mat& rotated_image) {
    int r=image.rows;
    int c=image.cols;
    for(int i=0;i<r;i++) {
        for(int j=0;j<c;j++) {
            rotated_image.at<Vec3b>(j,r-i-1)=image.at<Vec3b>(i,j);
        }
    }
}

int main(int argc, char* argv[]) {
    const char* input_file=argv[1];
    Mat image=imread(input_file, IMREAD_COLOR);
    int wf=atoi(argv[2]);
    int hf=atoi(argv[3]);
    if(image.empty()) {
        perror("Error opening image");
    }
    int w=image.cols;
    int h=image.rows;
    cout<<w<<" "<<h<<endl;
    // cout<<"Enter the final dimensions of the image: "<< endl;
    // int wf, hf;
    // cin >> wf;
    // cin >> hf;
    int kc=w-wf;
    int kr=h-hf;
    Mat carved_image=image;
    for(int i=0;i<kc;i++) {
        int wc=carved_image.cols;
        int hc=carved_image.rows;
        int** energy_mat=new int*[hc];
        for(int j=0;j<hc;j++) {
            energy_mat[j]=new int[wc];
        }
        calculate_energy_mat(carved_image, energy_mat);
        int* seam=calculate_seam(energy_mat, hc, wc);
        carved_image=carve_seam(carved_image, seam);
        for(int j=0;j<hc;j++) {
            delete[] energy_mat[j];
        }
        delete[] energy_mat;
        delete[] seam;
    }
    Mat rotated_image(carved_image.cols,carved_image.rows,carved_image.type());
    left_rotate(carved_image, rotated_image);
    carved_image=rotated_image;
    for(int i=0;i<kr;i++) {
        int wc=carved_image.cols;
        int hc=carved_image.rows;
        int** energy_mat=new int*[hc];
        for(int j=0;j<hc;j++) {
            energy_mat[j]=new int[wc];
        }
        calculate_energy_mat(carved_image, energy_mat);
        int* seam=calculate_seam(energy_mat, hc, wc);
        carved_image=carve_seam(carved_image, seam);
        for(int j=0;j<hc;j++) {
            delete[] energy_mat[j];
        }
        delete[] energy_mat;
        delete[] seam;
    }
    Mat final_image(carved_image.cols, carved_image.rows, carved_image.type());
    right_rotate(carved_image, final_image);
    int finalc=final_image.cols;
    int finalr=final_image.rows;
    cout<<finalc<<" "<<finalr<<endl;
    imshow("Original image",image);
    imshow("carved_image",final_image);
    waitKey(0);
    destroyAllWindows();
    return 0;
}