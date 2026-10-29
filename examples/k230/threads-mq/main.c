#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* RT-Smart is POSIX-ish (musl + pthreads over RT-Thread). The queue is built from pthread primitives so it is
 * portable; the native RT-Thread mq API is kernel-side and not used from user space here. */
#define QCAP 8
typedef struct { int id; char text[24]; } msg_t;
typedef struct {
    msg_t buf[QCAP]; int head, tail, count;
    pthread_mutex_t lock; pthread_cond_t not_empty, not_full;
} mq_t;

static mq_t q = { .lock = PTHREAD_MUTEX_INITIALIZER, .not_empty = PTHREAD_COND_INITIALIZER, .not_full = PTHREAD_COND_INITIALIZER };

static void mq_send(mq_t *m, const msg_t *x)
{
    pthread_mutex_lock(&m->lock);
    while (m->count == QCAP) pthread_cond_wait(&m->not_full, &m->lock);
    m->buf[m->tail] = *x; m->tail = (m->tail + 1) % QCAP; m->count++;
    pthread_cond_signal(&m->not_empty);
    pthread_mutex_unlock(&m->lock);
}
static void mq_recv(mq_t *m, msg_t *x)
{
    pthread_mutex_lock(&m->lock);
    while (m->count == 0) pthread_cond_wait(&m->not_empty, &m->lock);
    *x = m->buf[m->head]; m->head = (m->head + 1) % QCAP; m->count--;
    pthread_cond_signal(&m->not_full);
    pthread_mutex_unlock(&m->lock);
}

static void *producer(void *arg)
{
    (void)arg;
    for (int i = 0; i < 10; i++) {
        msg_t m = { .id = i };
        snprintf(m.text, sizeof m.text, "message %d", i);
        mq_send(&q, &m);
        usleep(200 * 1000);
    }
    msg_t end = { .id = -1 };
    mq_send(&q, &end);
    return NULL;
}
static void *consumer(void *arg)
{
    (void)arg;
    for (;;) {
        msg_t m; mq_recv(&q, &m);
        if (m.id < 0) break;
        printf("consumer: got #%d \"%s\"\n", m.id, m.text);
    }
    return NULL;
}

int main(void)
{
    pthread_t p, c;
    printf("threads-mq: start\n");
    pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    printf("threads-mq: done\n");
    return 0;
}
