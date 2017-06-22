void del_substr(char *str, const char *substr) {
  for (int i = 0, j = 0; *str = *(str+i); str += !!*(substr+j), i += !*(substr+j) * j)
    for (j = 0; *(str+i+j) == *(substr+j) && *(substr+j); j++);
}