#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// Address binding and port for tangle-accelerator
#define TA_HOST "localhost"
#define TA_PORT "8000"
#define TA_THREAD_COUNT 10

// IRI connection setting
#define IRI_HOST "localhost"
#define IRI_PORT 14265

// Redis connection setting
#define REDIS_HOST "localhost"
#define REDIS_PORT 6379

#define SEED                                                                   \
  "AMRWQP9BUMJALJHBXUCHOD9HFFD9LGTGEAWMJWWXSDVOF9PI9YGJAPBQLQUOMNYEQCZPGCTHGV" \
  "NNAPGHA"

#ifdef __cplusplus
}
#endif

#endif  // CONFIG_H_
