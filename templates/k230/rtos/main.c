#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define QCAP 8
static int buf[QCAP], head, tail, count;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER, not_full = PTHREAD_COND_INITIALIZER;

static void q_put(int v)
{
    pthread_mutex_lock(&lock);
    while (count == QCAP) pthread_cond_wait(&not_full, &lock);
    buf[tail] = v; tail = (tail + 1) % QCAP; count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}
static int q_get(void)
{
    pthread_mutex_lock(&lock);
    while (count == 0) pthread_cond_wait(&not_empty, &lock);
    int v = buf[head]; head = (head + 1) % QCAP; count--;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return v;
}

static void *producer(void *a) { (void)a; for (int i = 0; i < 5; i++) { q_put(i); usleep(300 * 1000); } q_put(-1); return NULL; }
static void *consumer(void *a) { (void)a; for (int v; (v = q_get()) >= 0;) printf("{{NAME}}: got %d\n", v); return NULL; }

int main(void)
{
    pthread_t p, c;
    pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    return 0;
}
