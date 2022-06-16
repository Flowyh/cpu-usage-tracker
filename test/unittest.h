#ifndef UNITTEST_H
#define UNITTEST_H

#define RUN_TEST(func) \
  do { \
    printf("========= RUNNING TEST: %s =========\n", #func); \
    (void)func(); \
    printf("========== END OF TEST: %s =========\n\n", #func); \
  } while(0)

#endif // !UNITTEST_H