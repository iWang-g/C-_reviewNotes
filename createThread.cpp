#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void task()
{
    cout << "�߳�����ִ������..." << endl;

    // �õ�ǰ�߳�����2��
    this_thread::sleep_for(chrono::seconds(2));

    cout << "�߳����߽���" << endl;
}

int main()
{
    thread t(task); // �����߳�
    t.join(); // �ȴ��߳̽���

    return 0;
}