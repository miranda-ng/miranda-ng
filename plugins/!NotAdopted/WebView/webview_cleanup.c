/*
 * A plugin for Miranda IM which displays web page text in a window Copyright 
 * (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
 * Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59 
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 */

/************************/
void            CodetoSymbol(char *truncated)
{
   int             counter = 0;
   int             position = 0, recpos = 0, n;
   static char    *stringfrompos;

   for (n = 0; n < AMOUNT3; n++)
   {

      while (1) // loop forever

      {
         Sleep(1); // avoid 100% CPU

         if (((strstr(truncated, CharacterCodes[n])) != 0))
// does character code exist?
         {

            if (strstr(truncated, CharacterCodes[n]) != 0)
            {
               stringfrompos = strstr(truncated, CharacterCodes[n]);
            }
            position = stringfrompos - truncated;
            counter = 0;
            while (counter != (strlen(CharacterCodes[n])))
            {
               truncated[position + counter] = ' ';
               counter++;
            }

            truncated[(position + counter) - 1] = Characters[n];

// 
            strncpy(&truncated[position], &truncated[position + strlen(CharacterCodes[n])] - 1, strlen(&truncated[position]) - 1);
// 

         } // end does character code exist?

         if (recpos == position)
            break; // break out of loop if doesn't find new character code

         recpos = position;
      } // end loop forever         

   } // for

}

/**********************/

/************************/
void            EraseBlock(char *truncated)
{
   int             counter = 0;
   int             positionStart = 0, positionEnd = 0;
   char           *stringfrompos;
   char           *tempraw;
   int             BlockLength = 0;

// //////

   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

// ///////////////////////////

// 1//

   while (1)
   {
      Sleep(1); // avoid 100% CPU
      // /get start and end of block

      if (strstr(tempraw, "<!--") != 0) // does tag exist?

      {
         stringfrompos = strstr(tempraw, "<!--");
         positionStart = stringfrompos - tempraw;
      }
      if (strstr(tempraw, "-->") != 0) // does tag exist?

      {
         stringfrompos = strstr(tempraw, "-->");
         positionEnd = stringfrompos - tempraw;
      }
      BlockLength = (positionEnd - positionStart) + 3;

      if ((strstr(tempraw, "<!--") == 0) || (strstr(tempraw, "-->") == 0))
         break;

/////////////////////////////////////////

      // /////////    
      if (strstr(tempraw, "<!--") != 0)
      {
         for (counter = 0; counter < BlockLength; counter++)
         {
            tempraw[positionStart + counter] = ' ';
         }
      }
      // //////////    

      if ((positionStart == 0) && (positionEnd == 0))
         break;
      if (positionStart > positionEnd)
         break;

      positionStart = 0;
      positionEnd = 0;
   }

// ///////////////////////////
   positionStart = 0;
   positionEnd = 0;

// 2//
   while (1)
   {
      Sleep(1); // avoid 100% CPU
      // /get start and end of block

      if (((strstr(tempraw, "<script")) != 0) || (strstr(tempraw, "<SCRIPT") != 0)) // does 
                                                                                    // 
         // tag 
         // exist?

      {
         if (strstr(tempraw, "<script") != 0)
            stringfrompos = strstr(tempraw, "<script");
         else
            stringfrompos = strstr(tempraw, "<SCRIPT");

         positionStart = stringfrompos - tempraw;
      }
      if (((strstr(tempraw, "</script")) != 0) || (strstr(tempraw, "</SCRIPT") != 0)) // does 
                                                                                      // 
         // tag 
         // exist?

      {
         if (strstr(tempraw, "<script") != 0)
            stringfrompos = strstr(tempraw, "</script");
         else
            stringfrompos = strstr(tempraw, "</SCRIPT");

         positionEnd = stringfrompos - tempraw;
      }
      BlockLength = (positionEnd - positionStart) + 9;

      if ((strstr(tempraw, "<script") != 0) && (strstr(tempraw, "</script") == 0))
         break;

      if ((strstr(tempraw, "<SCRIPT") != 0) && (strstr(tempraw, "</SCRIPT") == 0))
         break;

///////////////////////////////////////

      // /////////    

      if (((strstr(tempraw, "<script")) != 0) || ((strstr(tempraw, "<SCRIPT")) != 0))
      {
         for (counter = 0; counter < BlockLength; counter++)
         {
            tempraw[positionStart + counter] = ' ';
         }
      }
      // //////////    

      if ((positionStart == 0) && (positionEnd == 0))
         break;
      if (positionStart > positionEnd)
         break;

      positionStart = 0;
      positionEnd = 0;
   }

// ////
// 3//
   while (1)
   {
      Sleep(1); // avoid 100% CPU
      // /get start and end of block

      if (((strstr(tempraw, "<style")) != 0) || (strstr(tempraw, "<STYLE") != 0)) // does 
                                                                                    // 
         // tag 
         // exist?

      {
         if (strstr(tempraw, "<style") != 0)
            stringfrompos = strstr(tempraw, "<style");
         else
            stringfrompos = strstr(tempraw, "<STYLE");

         positionStart = stringfrompos - tempraw;
      }
      if (((strstr(tempraw, "</style")) != 0) || (strstr(tempraw, "</STYLE") != 0)) // does 
                                                                                      // 
         // tag 
         // exist?

      {
         if (strstr(tempraw, "<style") != 0)
            stringfrompos = strstr(tempraw, "</style");
         else
            stringfrompos = strstr(tempraw, "</STYLE");

         positionEnd = stringfrompos - tempraw;
      }
      BlockLength = (positionEnd - positionStart) + 8;

      if ((strstr(tempraw, "<style") != 0) && (strstr(tempraw, "</style") == 0))
         break;

      if ((strstr(tempraw, "<STYLE") != 0) && (strstr(tempraw, "</STYLE") == 0))
         break;

///////////////////////////////////////

      // /////////    

      if (((strstr(tempraw, "<style")) != 0) || ((strstr(tempraw, "<STYLE")) != 0))
      {
         for (counter = 0; counter < BlockLength; counter++)
         {
            tempraw[positionStart + counter] = ' ';
         }
      }
      // //////////    

      if ((positionStart == 0) && (positionEnd == 0))
         break;
      if (positionStart > positionEnd)
         break;

      positionStart = 0;
      positionEnd = 0;
   }

// ////




   // 4//
    /**/

      while (1)
   {
      Sleep(1); // avoid 100% CPU
      // /get start and end of block

      if (strstr(tempraw, "{") != 0) // does tag exist?

      {
         stringfrompos = strstr(tempraw, "{");
         positionStart = stringfrompos - tempraw;
      }
      if (strstr(tempraw, "}") != 0) // does tag exist?

      {
         stringfrompos = strstr(tempraw, "}");
         positionEnd = stringfrompos - tempraw;
      }
      BlockLength = (positionEnd - positionStart) + 1;

      if ((strstr(tempraw, "}") == 0) || (strstr(tempraw, "{") == 0))
         break;

/////////////////////////////////////////

      // /////////    
      if (strstr(tempraw, "{") != 0)
      {
         for (counter = 0; counter < BlockLength; counter++)
         {
            tempraw[positionStart + counter] = ' ';
         }
      }
      // //////////    

      if ((positionStart == 0) && (positionEnd == 0))
         break;
      if (positionStart > positionEnd)
         break;

      positionStart = 0;
      positionEnd = 0;
   }

// ///////////////////////////
   positionStart = 0;
   positionEnd = 0;

    /**/
      strncpy(truncated, tempraw, strlen(truncated));

   free(tempraw);
}

/************************/

/************************/
void            EraseSymbols(char *truncated)
{
   int             counter = 0;
   int             position = 0, recpos = 0;
   char           *stringfrompos;
   char           *tempraw;

   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

// //////

   while (1)
   {
      Sleep(1); // avoid 100% CPU

      /**/
         counter = 0;
      if ((strstr(tempraw, "&#")) != 0)
      {
         stringfrompos = strstr(tempraw, "&#");
         position = stringfrompos - tempraw;
         // 
         while (1)
         {
            tempraw[position + counter] = ' ';
            counter++;
            if (counter > 20)
               break;
            if (tempraw[position + counter] == ' ')
               break;
         }
         if (tempraw[position + counter] == ';')
         {
            tempraw[position + counter] = ' ';
         }
         // 
      }
      if (recpos == position)
         break;
      recpos = position;
       /**/
   }

// ////

   strncpy(truncated, tempraw, strlen(truncated));

   free(tempraw);
}

/************************/

/************************/
void            NumSymbols(char *truncated)
{
   int             counter = 0;
   int             position = 0, recpos = 0;
   char           *stringfrompos;
   char            symbol[20];
   int             character;
   char           *tempraw;

   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

   while (1)
   {
      Sleep(1); // avoid 100% CPU

      /**/
         counter = 0;

      if ((strstr(tempraw, "&#")) != 0)
      {
         stringfrompos = strstr(tempraw, "&#");
         position = stringfrompos - tempraw;

         // 
         while (1)
         {
            if (counter > 1)
               symbol[counter - 2] = tempraw[position + counter];

            tempraw[position + counter] = ' ';
            counter++;
            if (counter > 20)
               break;
            if ((tempraw[position + counter] == ';'))
            {
               symbol[counter - 2] = '\0';
               character = atoi(symbol);

               if (character > 0 && character < 256)
               {
                  memset(&tempraw[position], character, 1);
               }
               break;
            }
         }

         if (tempraw[position + counter] == ';')
         {
            tempraw[position + counter] = ' ';

         }
         // 
      }
      if (recpos == position)
         break;
      recpos = position;
       /**/
   }

// ////

   strncpy(truncated, tempraw, strlen(truncated));
   free(tempraw);
}

/************************/

/************************/
void            FastTagFilter(char *truncated)
{
   int             counter = 0;
   char           *tempraw;

// //////
   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

   for (counter = 0; counter < lstrlen(tempraw); counter++)
   {
      if (tempraw[counter] == '<')
      {
         while (tempraw[counter] != '>')
         {
            if (counter >= lstrlen(tempraw))
               break;
            tempraw[counter] = ' ';
            counter++;
         }
         if (tempraw[counter] == '>')
            tempraw[counter] = ' ';
      }
   }

   strncpy(truncated, tempraw, strlen(truncated));
   free(tempraw);
}
/*************************/

/********************/
void            RemoveInvis(char *truncated, int AmountWspcRem)
{
   int             counter = 0;
   int             erase = 0;
   int             RemovalLevel = 0;
   char           *tempraw;

   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

   switch (AmountWspcRem)
   {

   case 1:
      RemovalLevel = 80; // small

      break;
   case 2:
      RemovalLevel = 30; // medium

      break;
   case 3:
      RemovalLevel = 10; // large

      break;
   default:
      break;

   }

// //////
   for (counter = 0; counter < lstrlen(tempraw); counter++)
   {

      // 
      if (AmountWspcRem != 0 && AmountWspcRem != 4)
      {
         if ((tempraw[counter] == '\n') || (tempraw[counter] == ' ') || (tempraw[counter] == '\r'))
            erase = erase + 1;
         else
            erase = 0;

         if (erase > RemovalLevel)
            tempraw[counter] = ' ';
      }
      // 
      if (AmountWspcRem == 4)
      {
         if ((tempraw[counter] == '\n') || (tempraw[counter] == ' ') || (tempraw[counter] == '\r'))
            tempraw[counter] = ' ';
      }
      // 

   } // end for
   // //////

   strncpy(truncated, tempraw, strlen(truncated));
   free(tempraw);
}

/******************/

/******************/
void            RemoveTabs(char *truncated)
{
   int             counter = 0;
   char           *tempraw;

   tempraw = malloc(MAXSIZE1);

   if (truncated)
      strncpy(tempraw, truncated, MAXSIZE1);

   for (counter = 0; counter < lstrlen(tempraw); counter++)
   {

      if (tempraw[counter] == '\t')
         tempraw[counter] = ' ';

   } // end for

   strncpy(truncated, tempraw, strlen(truncated));

   free(tempraw);
}

/******************/

/********************/
void            Removewhitespace(char *truncated)
{
   int             counter = 0;
   int             counter2 = 0;
   int             pos1 = 0, pos2 = 0;

   for (counter = 0; counter < lstrlen(truncated); counter++)
   {
      if (truncated[counter] == ' ' && truncated[counter + 1] == ' ')
      {
         pos1 = counter + 1;
         counter2 = counter;

         while (truncated[counter2] == ' ')
         {
            counter2++;
         }

         pos2 = counter2;
         strncpy(&truncated[pos1], &truncated[pos2], strlen(&truncated[pos1]) - 1);

      } // end if

   } // end for

}

/******************/

/********************/
void            Filter(char *truncated)
{
   int             counter = 0;
   char            tempraw[MAXSIZE1];

   strncpy(tempraw, truncated, sizeof(tempraw));

   for (counter = 0; counter < lstrlen(tempraw); counter++)
   {

      if ((tempraw[counter] == '\n') || (tempraw[counter] == '\r') || (tempraw[counter] == '\t'))
      {
         strncpy(&tempraw[counter], &tempraw[counter + 1], strlen(&tempraw[counter]) - 1);
      }
   } // end for

   strncpy(truncated, tempraw, strlen(truncated));

}

/******************/
