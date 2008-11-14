#include "write_test.h"

char* header1 =
"# Data file for mpc_fr_div.\n"
"#\n"
"# Copyright (C) 2008 Andreas Enge\n"
"#\n"
;
char* header2 =
"# For explanations on the file format, see add.dat, and take into\n"
"# account the necessary modifications for one real argument.\n\n"
;

int main () {
   int op1i, op2rei, op2imi;
      // index in values table
   char *op1, *op2re, *op2im;

   printf ("%s%s%s", header1, licence, header2);

   printf ("# special values\n");
   for (op1i = 0; op1i < valueslen; op1i++)
   for (op2rei = 0; op2rei < valueslen; op2rei++)
   for (op2imi = 0; op2imi < valueslen; op2imi++)
   {
      op1 = values [op1i];
      op2re = values [op2rei];
      op2im = values [op2imi];
      if (   (index (infvalues, infvalueslen, op1) != -1)
          || (index (infvalues, infvalueslen, op2re) != -1)
          || (index (infvalues, infvalueslen, op2im) != -1))
      {
         // at least one infinite value
         printf ("7 %4s 7 %4s   ",
            div_re (op1, "+0", op2re, op2im),
            div_im (op1, "+0", op2re, op2im));
         printf ("7 %4s  7 %4s 7 %4s N N\n",
            op1, op2re, op2im);
      }
   }

   printf ("\n# finite values\n");
   for (op1i = 0; op1i < valueslen; op1i++)
   for (op2rei = 0; op2rei < valueslen; op2rei++)
   for (op2imi = 0; op2imi < valueslen; op2imi++)
   {
      op1 = values [op1i];
      op2re = values [op2rei];
      op2im = values [op2imi];
      if (   (index (infvalues, infvalueslen, op1) == -1)
          && (index (infvalues, infvalueslen, op2re) == -1)
          && (index (infvalues, infvalueslen, op2im) == -1))
      {
         printf ("7 %4s 7 %4s   ",
            div_re (op1, "+0", op2re, op2im),
            div_im (op1, "+0", op2re, op2im));
         printf ("7 %4s  7 %4s 7 %4s N N\n",
            op1, op2re, op2im);
      }
   }

   return 1;
}
