#include <stdio.h>
#include <stdbool.h>

unsigned int pht1level [65536];
unsigned int pht2level [65536];
unsigned int gshare [65536];
unsigned int phtlocal [65536];
unsigned int choosertable [65536];

struct globalpattern {
  unsigned int ghr:16;
}pattern, pattern1, localpattern [65536];


void phtinit(void);
bool onelevelpred(unsigned long int addr, char a);
bool l2globalmispred (char a);
bool gshared (unsigned long int address, char a);
bool localhistory (unsigned long int address, char a);
bool tournamentchooser (unsigned long int address, char a, bool localpred, bool gsharepred);


void main()
{
  FILE *fp, *fp1;
  int i, type;
  unsigned long int address, instr;
  unsigned int masked_addr;
  char a;

  phtinit();  

  // open trace file for reading

  fp = fopen("branch-trace-gcc-file.trace", "r");
  int l1mispred = 0;
  int l2mispred = 0;
  int gsharemispred = 0;
  int localmispred = 0;
  int tourmispred = 0;
 

  // read first 100 lines of trace file; 
  // address holds virtual address in unsigned int format;
  // N.B. - your program must read all 1,000,000 lines but
  //        you might want to test it on the first 100 lines
  //        or so before running it on the entire trace file!
  while(!feof (fp))
  { 

    // read 1 line of trace file

    fscanf(fp, "%ld %c", &address, &a);

    // output virtual address in hex 

    //printf("%ld %c\n", address,a);
    if (!onelevelpred(address, a))
      l1mispred++;
    if (!l2globalmispred(a))
      l2mispred++;
    if (!gshared(address, a))
      gsharemispred++;
    if (!localhistory(address, a))
      localmispred++;
  }
  fclose(fp);
  

  fp1 = fopen("branch-trace-gcc-file.trace", "r");
  phtinit();
  while(!feof (fp1))
  {
    fscanf(fp1, "%ld %c", &address, &a);

    if (!tournamentchooser(address, a, localhistory(address, a), gshared(address,a)))
      tourmispred++;

  }

  fclose(fp1);
  float l1, l2, gshr, loc, tour;
  l1 = l1mispred*100.0/16416279;
  l2 = l2mispred*100.0/16416279;
  gshr = gsharemispred*100.0/16416279;
  loc = localmispred*100.0/16416279;
  tour = tourmispred*100.0/16416279;

  printf("%d %f\n%d %f\n%d %f\n%d %f\n%d %f\n", l1mispred, l1, l2mispred, l2, gsharemispred, gshr, localmispred, loc, tourmispred, tour);

}

bool onelevelpred(unsigned long int address, char a)
{
  int index = (address >> 2) % 65536;
  bool prediction;

  if (pht1level[index] <= 3)
  {
    if (a == 'T')
    {
      pht1level[index]++;
      prediction = false;

    }
    else 
      if (a == 'N')
      {
        if (pht1level[index] > 0)
            pht1level[index]--;

        prediction = true;
      }

  }
  else if (pht1level[index] > 3)
  {
    if (a == 'N')
    {
      pht1level[index]--;
      prediction = false;
    }
    else 
      if (a == 'T')
      {
        if(pht1level[index] < 7)
          pht1level[index]++;
        prediction = true;
      }
  }  
  return prediction;

}

void phtinit()
{

  int i;

  for (i=0;i<65536;i++)
  {
    pht1level[i] = 0;
  }
  for (i=0;i<65536;i++)
  {
    pht2level[i] = 0;
  }
  for (i=0;i<65536;i++)
  {
    gshare[i] = 0;
  }
  for (i=0;i<65536;i++)
  {
    phtlocal[i] = 0;
  }
  for (i=0;i<65536;i++)
  {
    choosertable[i] = 0;
  }



  pattern.ghr = 0;


}

bool l2globalmispred (char a)
{

  int index = pattern.ghr;
  //index%=1024;
  char status;
  if (a=='T')
    status = 1;
  if (a=='N')
    status = 0;

  pattern.ghr = pattern.ghr << 1 | status;

  //printf ("%d\n", status);

  bool prediction;

  if (pht2level[index] <= 3)
  {
    if (a == 'T')
    {
      pht2level[index]++;
      prediction = false;

    }
    else 
      if (a == 'N')
      {
        if (pht2level[index]>0)
            pht2level[index]--;

        prediction = true;
      }

  }
  else if (pht2level[index] > 3)
  {
    if (a == 'N')
    {
      pht2level[index]--;
      prediction = false;
    }
    else 
      if (a == 'T')
      {
        if(pht2level[index] < 7)
          pht2level[index]++;
        prediction = true;
      }
  }  
  return prediction;



}

bool gshared (unsigned long int address, char a)
{

  int index = pattern1.ghr;
  //index%=1024;
  char status;
  if (a=='T')
    status = 1;
  if (a=='N')
    status = 0;

  pattern1.ghr = pattern1.ghr << 1 | status;

  index = 0xFFFF & (index ^ ((address) & 0xFFFF));


  bool prediction;

  if (gshare[index] <= 3)
  {
    if (a == 'T')
    {
      gshare[index]++;
      prediction = false;

    }
    else 
      if (a == 'N')
      {
        if (gshare[index]>0)
            gshare[index]--;

        prediction = true;
      }

  }
  else if (gshare[index] >3)
  {
    if (a == 'N')
    {
      gshare[index]--;
      prediction = false;
    }
    else 
      if (a == 'T')
      {
        if(gshare[index] < 7)
          gshare[index]++;
        prediction = true;
      }
  }  
  return prediction;



}

bool localhistory (unsigned long int address, char a)
{

  int localindex = (address) % 65536;

  int index = localpattern[localindex].ghr;

  char status;
  if (a=='T')
    status = 1;
  if (a=='N')
    status = 0;

  localpattern[localindex].ghr = localpattern[localindex].ghr << 1 | status;

  bool prediction;

  if (phtlocal[index] <=3)
  {
    if (a == 'T')
    {
      phtlocal[index]++;
      prediction = false;

    }
    else 
      if (a == 'N')
      {
        if (phtlocal[index]>0)
            phtlocal[index]--;

        prediction = true;
      }

  }
  else if (phtlocal[index] >3)
  {
    if (a == 'N')
    {
      phtlocal[index]--;
      prediction = false;
    }
    else 
      if (a == 'T')
      {
        if(phtlocal[index] < 7)
          phtlocal[index]++;
        prediction = true;
      }
  }  
  return prediction;

}


bool tournamentchooser (unsigned long int address, char a, bool localpred, bool gsharepred)
{
  int tour_index = address % 65536;

  if (choosertable [tour_index] <= 3)
  {
    if (!localpred)
    {
      choosertable [tour_index]++;
      return false;
    }
    else
    {
      if (choosertable[tour_index]>0)
        choosertable [tour_index]--;
      return true;
    }
  }
  else
    if (choosertable [tour_index] > 3)
  {
    if (!gsharepred)
    {
      choosertable [tour_index]--;
      return false;
    }
    else
    {
      if (choosertable[tour_index] < 7)
        choosertable [tour_index]++;
      return true;
    }
  }
}



