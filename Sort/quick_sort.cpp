#include <iostream>
using namespace std;

void quick_sort1(int arr[], int l, int r)
{
    if(l < r)
    {
        int i = l, j = r, temp = arr[l];
        while(i < j)
        {
            while(i < j)
            {
                if(arr[j] <= temp) {
                    arr[i] = arr[j];
                    i++;
                    break;
                }
                j--;
            }

            while(i < j)
            {
                if(arr[i] > temp) {
                    arr[j] = arr[i];
                    j--;
                    break;
                }
                i++;
            }
        }
        arr[i] = temp;
        quick_sort1(arr, l, i - 1);
        quick_sort1(arr, i + 1, r);
    }
}

void quick_sort2(int arr[], int l, int r)
{
    if(l < r)
    {
        swap(arr[l], arr[(l + r) / 2]);
        int i = l, j = r, temp = arr[l];
        while(i < j)
        {
            while(i < j && arr[j] >= temp)
                j--;
            if(i < j)
                arr[i++] = arr[j];

            while(i < j && arr[i] < temp)
                i++;
            if(i < j)
                arr[j--] = arr[i];
        }
        arr[i] = temp;
        quick_sort2(arr, l, i - 1);
        quick_sort2(arr, i + 1, r);
    }
}

int main()
{
    int arr[] = {2, 1, 5, 4, 3};
    quick_sort2(arr, 0, 4);
    for(int x : arr)
    {
        cout << x << " ";
    }
    return 0;
}