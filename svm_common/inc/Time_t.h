#ifndef  __TIME_T_H__
#define  __TIME_T_H__

#ifdef __cplusplus
extern "C" {
#endif
  /*
      usage: start -> time_stamp(0, NULL)
             end   -> time_stamp(1, "name")
  */
  void   time_stamp(int is_end, const char *timeName);

#ifdef __cplusplus
}
#endif

#endif
