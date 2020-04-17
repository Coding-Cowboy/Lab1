#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>

static const long Num_To_Sort = 100000000;//this will be n

//Sorting references can be found from https://www.geeksforgeeks.org/quick-sort/
//and https://stackoverflow.com/questions/41346061/parallel-sorting-in-quick-sort-using-multi-threads-in-c
//Swapping function
void swap(int *a,int *b)
{
    int t= *a;
    *a= *b;
    *b = t;
}
void checkArray(int *arr)
{
    for(int i=0;i<Num_To_Sort-1;i++)
    {
        if(arr[i]>arr[i+1])
            printf("%d ",arr[i]);//only prints if they are out of order
    }
    printf("\n");
}
int partition_s(int *arr, int low, int high)
{
    int pivot = arr[high];//pivot for where to start sort
    int i = low-1; //Index of smaller element

    for(int j= low; j <=high-1; j++)
    {
        //if current is smaller than pivot
        if(arr[j] < pivot)
        {
            i++; //increment index
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i+1], &arr[high]);
    return(i+1);
}

//This is the recursive quicksort algorithm that will be used for the sequencial quicksort
void quickSort_s(int *arr,int low, int high)
{
    if(low<high)
    {
        int pi = partition_s(arr,low,high);

        quickSort_s(arr, low,pi-1);// before Partition index
        quickSort_s(arr,pi+1,high);// after Partition index
    }
}
// Sequential version of your sort
// If you're implementing the PSRS algorithm, you may ignore this section
void sort_s(int *arr) {
    quickSort_s(arr, 0, Num_To_Sort-1);
}

int partition_p(int arr[], int low, int high)
{
    int pivot = arr[high];//pivot for where to start sort
    int i = low-1; //Index of smaller element
    int chunkSize = (high-1)/omp_get_num_threads();
    int localI= i;
//    int startI=low+(chunkSize*omp_get_thread_num());
//    int endI=((high-1)-startI+chunkSize);
#pragma omp for\
reduction(+:localI) schedule(static,chunksize)
    for(int j=low; j <=(high-1); j++)
    {
        //if current is smaller than pivot
        if(arr[j] < pivot)
        {
            localI++; //increment index
            swap(&arr[i], &arr[j]);
        }
    }
    i+localI;
    swap(&arr[i+1], &arr[high]);
    return(i+1);
}
void quickSort_p(int arr[],int low, int high)
{
    if(low<high)
    {
        int pi = partition_s(arr,low,high);
#pragma omp single nowait
        quickSort_p(arr, low,pi-1);// before Partition index
#pragma omp single nowait
        quickSort_p(arr,pi+1,high);// after Partition index
    }
}
// Parallel version of your sort
//This will parallelizes the partition portion of the quicksort to help speed the process along
void sort_p(int *arr) {
# pragma omp parallel num_threads(omp_get_max_threads())
#pragma omp single
    quickSort_p(arr, 0, Num_To_Sort - 1);
}

int main() {
    int *arr_s = malloc(sizeof(int) * Num_To_Sort);
    long chunk_size = Num_To_Sort / omp_get_max_threads();
#pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;
        for (long i = chunk_start; i < chunk_end; i++) {
            arr_s[i] = rand_r(&seed);
        }
    }

    // Copy the array so that the sorting function can operate on it directly.
    // Note that this doubles the memory usage.
    // You may wish to test with slightly smaller arrays if you're running out of memory.
    int *arr_p = malloc(sizeof(int) * Num_To_Sort);
    memcpy(arr_p, arr_s, sizeof(int) * Num_To_Sort);

    struct timeval start, end;

    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    sort_s(arr_s);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    free(arr_s);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    sort_p(arr_p);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);
    checkArray(arr_p);
    free(arr_p);

    return 0;
}

