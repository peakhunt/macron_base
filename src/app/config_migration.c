#include <stdio.h>
#include <stdlib.h>
#include "config_reader.h"
#include "cJSON_Utils.h"

int
main(int argc, char** argv)
{
  char          *left,
                *right;

  cJSON         *jleft,
                *jright,
                *jfinal;

  char          *print_buffer;
  if(argc != 3)
  {
    fprintf(stderr, "%s left right\n", argv[0]);
    exit(-1);
  }

  left  = argv[1];
  right = argv[2];

  jleft   = config_reader_init(left);
  jright  = config_reader_init(right);

  if(jleft == NULL || jright == NULL)
  {
    fprintf(stderr, "json parsing failed\n");
    exit(-1);
  }

  fprintf(stdout, "generating patch\n");
  jfinal = cJSONUtils_GenerateMergePatchCaseSensitive(jleft, jright);

  print_buffer = cJSON_Print(jfinal);
  fprintf(stdout, "%s", print_buffer);
  return 0;
}
