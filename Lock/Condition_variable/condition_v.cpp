#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex mtx;
condition_variable cv;
bool ready = false;

/*
    lock_guard vs unique_lock:
    lock_guard����������������һЩ��Ա������
    unique_lock����unlock�����������ֶ��ͷ����������������������unique_lock
    ʹ�ã�����������waitʱ��Ҫ���ֶ��ͷ�����������
*/
void worker_thread() {
    // �ȴ����̷߳���֪ͨ
    unique_lock<mutex> lock(mtx);
    cout << "�����߳̽���ȴ�״̬" << endl;
    cv.wait(lock, []{
        return ready;
    });
    cout << "���յ����̷߳��͵�֪ͨ" << endl;
    cout << "Worker thread is processing..." << std::endl;
}

/*
    1. �����̵߳���cv.wait()����ȴ�״̬
    2. ���߳���׼��������ɺ�����ready=true������cv.notify_one()
    3. �����̱߳����ѣ��������readyΪ������ִ��
*/
void test_worker_thread() {
    thread worker(worker_thread);

    // ģ��׼������
    cout << "readying..." << std::endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << "ready finished" << std::endl;

    {
       lock_guard<mutex> lock(mtx);
       ready = true;
       cv.notify_one(); // ֪ͨ�ȴ����߳�
    }

    worker.join();
}

int main()
{
    test_worker_thread();
    return 0;
}