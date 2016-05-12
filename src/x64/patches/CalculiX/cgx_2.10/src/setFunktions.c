/* --------------------------------------------------------------------  */
/*                          CALCULIX                                     */
/*                   - GRAPHICAL INTERFACE -                             */
/*                                                                       */
/*     A 3-dimensional pre- and post-processor for finite elements       */
/*              Copyright (C) 1996 Klaus Wittig                          */
/*                                                                       */
/*     This program is free software; you can redistribute it and/or     */
/*     modify it under the terms of the GNU General Public License as    */
/*     published by the Free Software Foundation; version 2 of           */
/*     the License.                                                      */
/*                                                                       */
/*     This program is distributed in the hope that it will be useful,   */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of    */ 
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the      */
/*     GNU General Public License for more details.                      */
/*                                                                       */
/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software       */
/*     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.         */
/* --------------------------------------------------------------------  */

/* TO DO
MESHOPT_ANGLE is not working good enough. after the optimization elem-edges might cross. all element edges must be regarded in future.
*/
/*
Peter Heppel Jan 2016
    
   if keyword ndsb  (NoDeleteShellsBeams) is present in pre_read, 
   then DO NOT  delete elements in special sets created by readNG 


*/

#include <cgx.h>
#include <sys/dir.h>
//#include <X11/Xmd.h>
#include <xwd.h>
//#include <X11/XWDFile.h>
#include <pthread.h>


#define TEST 0


/* choose the optimization-steps for the mesher */
#define MESHOPT_LENGTH 1
#define MESHOPT_ANGLE  0
#define MAX_PARAM_IN_BRECORD 100
#define     N_CLOSEST_NODES 10
#define     N_CLOSEST_TRI 150
#define     SMALL_TET_VOLUME 1.e-20
#define     LTOL 1.e-7                 // tolerance for tet splitting

extern char entity_k[SET_COLS];  /* predefined colors of entities */
extern GLfloat entity_r[SET_COLS];                     /* predefined colors of entities */
extern GLfloat entity_g[SET_COLS];                     /* predefined colors of entities */
extern GLfloat entity_b[SET_COLS];                     /* predefined colors of entities */
extern char  datin[MAX_LINE_LENGTH];         /* Input-data-file */
extern char  buffer[MAX_LINE_LENGTH];
extern int   offset, maxIndex;                        /* offset+steps-1 = maxIndex */
extern int   basCol[3];                     /* color indexes due to basic colormap */
extern int   width_ini, height_ini; /* Grafig-Fensterbreite/hoehe */
extern int   width_menu, height_menu;
extern int   w0, w1, w_index, w_rgb, activWindow;
extern int   width_w0, height_w0;
extern int   width_w1, height_w1;
extern int   MouseMode;                                   /* status maustasten */
extern int   halfperiod;
extern double dtx, dty, dtz, drx, dry, drz, ds;                 /* Verschiebungen */
extern double vmem[4];                                     /* kor. bis auswahl eines neuen drehpkts */
extern double anim_faktor;                            /* Scalierung der Amplitude */
extern char  drawMode;               /* protokoliert drawFunktion (Load=1,Light=2,Animate=3,preprocessor=4, vector=5)*/
extern char  animFlag;
extern double lmodel_twoside[1];
extern GLint   gl_max_eval_order;                         /* max order of NURBS */
extern GLdouble R[4][4];                                   /* Rotationsmatrix */
extern char  surfFlag;                /* zeichne nur Oberflaechenelemente (1), sonst (0)*/
extern char  modelEdgeFlag;                /* zeichne mit Modell-Ecken (1), sonst (0)*/
extern char  elemEdgeFlag;                 /* zeichne mit Surface Ecken (1), sonst (0)*/
extern char  frameFlag;                   /* mit (1) oder ohne Rahmen um das Grafikfenster */
extern char  sequenceFlag;                  /* 1: play a sequence of LC */
extern char  filenamFlag;                   /* mit (1) oder ohne filename im Menufenster */
extern char  textFlag;                   /* mit (1) oder ohne text im Menufenster */
extern char  printFlag;                     /* printf 1:on 0:off */
extern char  frameFlag;               /* mit (1) oder ohne Rahmen um das Grafigfenster */
extern char  scalaFlag;                 /* mit (1) oder ohne scala und wertetexte */ 
extern char  addDispFlag;                    /* 0: original node-coordinates, 1: node-coordinates+displacements */
extern char  flipColorFlag;                 /* 0: high values use red, low use blue in scale; 1: flipped */
extern char  vectorFlag;                    /* 0: scalar plot, 1: vector plot */
extern double dx ,dy;                                      /* Mauskoordinaten */
extern int   steps;                          /* Schrittweite der Farbscala */
extern double gtol;
extern int     ddiv;
extern double     dbias;
extern char  picture_text[MAX_LINE_LENGTH], datin[MAX_LINE_LENGTH];
extern double v_scale;                                    /* scaling-factor for the vectors in the vector-plot */
extern char  psviewer[MAX_LINE_LENGTH];

extern char  delPntFlag;                    /* 1: deleted points exists */
extern char  delShapeFlag;                  /* 1: deleted shapes exists */
extern char  delLineFlag;                   /* 1: deleted lines exists */
extern char  delLcmbFlag;                   /* 1: deleted lcmbs exists */
extern char  delSurfFlag;                   /* 1: deleted surfs exists */
extern char  delBodyFlag;                   /* 1: deleted bodys exists */
extern char  delNursFlag;
extern char  delSetFlag;                  /* 1: deleted sets exists */
extern char  movieFlag;                     /* >0: save sequence of gif pictures */
extern int  movieFrames;                     
extern char  movieCommandFile[MAX_LINE_LENGTH];        /* stores the file name of a command file which will be executed after the movie is created (frames option only) */
extern char  stopFlag;                      /* stop/start animation */
extern char  inpformat;                     /* defines the start-up mode of cgx */

extern int     read_mode;
extern int     setall;
extern int   animList;
extern int   lcase_animList;

extern int     elemMat[MAX_MATERIALS];      /*  Material Numbers, Number of Materials stored in elemMat[0]  */

extern int       nasMpc;                                       /* 1: areampc generates mpcs; 0: rbes with optional heat-expansion-coefficient */
extern double    nasRbeHec; 

/* for rgb mode */
extern Colours   *colRgb;

/* for index mode */
extern int       *colNr;

extern Scale     scale[1];
extern Elements  *e_enqire;     /* elem-array by elem-number instead of elem[index]... */
extern Summen    anz[1];
extern Edges     *edge;
extern Nodes     *node;
extern Datasets *lcase;
extern Faces     *face;
extern NodeBlocks *nBlock;
extern BGpicture *bgpicture;

extern Meshp meshp;

extern Alias     *alias;
extern Sets      *set;
extern Shapes    *shape;
extern Materials *material; 
extern Psets     *pset;
extern Values    *value;
extern Points    *point;
extern Lines     *line;
extern Lcmb      *lcmb;
extern Gsur      *surf;
extern Gbod      *body;
extern Nurbl     *nurbl;
extern Nurbs     *nurbs;
extern Materials *material; 
extern Amplitudes *amplitude; 
extern SumGeo    anzGeo[1];
extern SumAsci   sumAsci[1];
extern Texts     *ntext;

/* nr of hardcopies */
extern int psNr, tgaNr, gifNr, pngNr;


extern Eqal eqal;

/* additional entities */
extern char **valuestack;
extern int valuestack_ptr, valuestackFlag;
extern OpenSets   openSets[1];
extern SpecialSet specialset[1];

Lchar     lchar[1]={{'0','0','D','L','C','A','B','Q','N','A','H'}};      /* first letter in the entity names */

extern int  cur_entity;                                       /* aktive entity (component) */
extern int  cur_lc;
extern void       *glut_font[];  /* glut fonts */
extern int       legend_font;                         /* active font for the legend */
extern int       draw_font;                         /* active font for the annotation of entities */
extern int       menu_font;                         /* active font for the menu */

/* the copied node-sets which have to be filled with values from new loaded Datasets */
extern CopiedNodeSets copiedNodeSets[1];

int       nodeCsys=0;                        /*  Number of coordinate systems (nastran)  */
int       nodeCsysSet[MAX_MATERIALS];        /*  setNr  */
int       nodeCsysNr[MAX_MATERIALS];         /*  csysNr  */

/* mpc generation between incompatible element-types for connected bodies */
int sum_equSets=0, *depSet=NULL, *indSet=NULL;


/* returns node-Index if known, or -(Index+10) of a unused node or if none is unused -1  */
int getNodNr(Summen *anz, Nodes *node, int nodnr )
{
  //int pfree;
  //printf("nodnr:%d anz->nmax:%d \n", nodnr,anz->nmax);
  if(nodnr>anz->nmax) return(-1);
  //printf("nodindx:%d anz->n:%d \n", node[nodnr].indx,anz->n);
  if(node[nodnr].indx<0) return(-1);
  if(node[nodnr].indx>anz->n) return(-1);

  if( node[node[nodnr].indx].nr == nodnr )
  {
    /* unused node ?  */
    //if( node[nodnr].pflag == -1) return(pfree=-node[nodnr].indx-10);
    if( node[nodnr].pflag == -1) return(-node[nodnr].indx-10);
    else return(node[nodnr].indx);
  }
  return(-1);
}



/* returns alias-Index if known, or if not -1  */
int getAliasNr(char *name)
{
  static int i, n, length, pfree, sum;

  if(!anzGeo->alias) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  if(sum<0)
  {
    printf ("ERROR: Illegal name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_suma)
  {
   for (i=0; i<sumAsci->anza[sum]; i++)
   {
    if(( alias[sumAsci->aindx[sum][i]].name != (char *)NULL ) && (strlen(alias[sumAsci->aindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(alias[sumAsci->aindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->aindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, alias[sumAsci->pindx[sum][i]].name, sumAsci->aindx[sum][i]);
      */
    }
   }
  }
  else
  {
    if(printFlag) printf(" WARNING alias:%s not found in hash-table\n", name);
     for (i=0; i<anzGeo->alias; i++) if( alias[i].name != (char *)NULL )
      if((compare( alias[i].name, name, length)==length) && (sword( alias[i].name, buffer)==length))
        return(i);
  }
  return(pfree);
}


/* returns value-Index if known, or -(Index+10) of a deleted value or if none is deleted -1  */
int getValuNr(char *name)
{
  static int i, n, length, pfree, sum;

  if(!anz->v) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal value name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumv)
  {
   for (i=0; i<sumAsci->anzv[sum]; i++)
   {
    if(( value[sumAsci->vindx[sum][i]].name != (char *)NULL ) && (strlen(value[sumAsci->vindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(value[sumAsci->vindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->vindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, value[sumAsci->vindx[sum][i]].name, sumAsci->vindx[sum][i]);
      */
    }
   }
  }
  else 
  {
    if(printFlag) printf(" WARNING value:%s not found in hash-table\n", name);
    for (i=0; i<anz->v; i++) if( value[i].name != (char *)NULL )
      if((compare( value[i].name, name, length)==length) && (sword( value[i].name, buffer)==length))
        return(i);
  }

  /* if not found  look for free cells */
  for (i=0; i<anz->v; i++)
  {
    /* do we have a "deleted" point for use?  */
    if( value[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  return(pfree);
}


/* returns pnt-Index if known, or -(Index+10) of a deleted pnt or if none is deleted -1  */
int getPntNr(char *name)
{
  static int i, n, length, pfree, sum;

  if(!anzGeo->p) return(-1);

  i=length=sum=0; 
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Point name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sump)
  {
   for (i=0; i<sumAsci->anzp[sum]; i++)
   {
    if(( point[sumAsci->pindx[sum][i]].name != (char *)NULL ) && (strlen(point[sumAsci->pindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(point[sumAsci->pindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->pindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, point[sumAsci->pindx[sum][i]].name, sumAsci->pindx[sum][i]);
      */
    }
   }
  }
  else 
  {
    if(printFlag) printf(" WARNING point:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->p; i++) if( point[i].name != (char *)NULL )
      if((compare( point[i].name, name, length)==length) && (sword( point[i].name, buffer)==length))
        return(i);
  }

  /* if not found  look for free cells */
  if (delPntFlag)
  for (i=0; i<anzGeo->p; i++)
  {
    /* do we have a "deleted" point for use?  */
    if( point[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delPntFlag=0; /* only if "deleted" was no more found */
  return(pfree);
}


/* returns shape-Index if known, or -(Index+10) of a deleted shape or if none is deleted -1  */
int getShapeNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->sh) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  /*
  printf ("in getShapeNr anzsh[%d]: %d\n", sum, sumAsci->anzsh[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal shape name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumsh)
  {
   for (i=0; i<sumAsci->anzsh[sum]; i++)
   {
    if(( shape[sumAsci->shindx[sum][i]].name != (char *)NULL ) && (strlen(shape[sumAsci->shindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(shape[sumAsci->shindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->shindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, shape[sumAsci->shindx[sum][i]].name, sumAsci->shindx[sum][i]);
      */
    }   
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING shape:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->sh; i++)
    {
      if( shape[i].name != (char *)NULL )
      {
        if((compare( shape[i].name, name, length)==length) && (sword( shape[i].name, buffer)==length))
          return(i);
      }
    }
  }

  /* if not found look for free cells */
  if (delShapeFlag)
  for (i=0; i<anzGeo->sh; i++)
  {
    /* do we have a "deleted" shape for use?  */
    if( shape[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  } 
  delShapeFlag=0;
  return(pfree);
}


/* returns line-Index if known, or -(Index+10) of a deleted line or if none is deleted -1  */
int getLineNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->l) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);

  /*
  printf ("in getLineNr anzl[%d]: %d\n", sum, sumAsci->anzl[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal line name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_suml)
  {
   for (i=0; i<sumAsci->anzl[sum]; i++)
   {
    if(( line[sumAsci->lindx[sum][i]].name != (char *)NULL ) && (strlen(line[sumAsci->lindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(line[sumAsci->lindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->lindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, line[sumAsci->lindx[sum][i]].name, sumAsci->lindx[sum][i]);
      */
    }   
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING line:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->l; i++)
    {
      if( line[i].name != (char *)NULL )
      {
        if((compare( line[i].name, name, length)==length) && (sword( line[i].name, buffer)==length))
          return(i);
      }
    }
  }

  /* if not found look for free cells */
  if (delLineFlag)
  for (i=0; i<anzGeo->l; i++)
  {
    /* do we have a "deleted" line for use?  */
    if( line[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  } 
  delLineFlag=0;
  return(pfree);
}


/* returns line-Index if known, or -(Index+10) of a deleted line or if none is deleted -1  */
int getLcmbNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->c) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  /* printf ("in getLcmbNr sum:%d max_sumc:%d anzGeo->c:%d\n", sum, sumAsci->max_sumc, anzGeo->c); */
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Lcmb name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumc)
  {
   /* printf ("in getLcmbNr anzc[%d]:%d \n", sum, sumAsci->anzc[sum]); */
   for (i=0; i<sumAsci->anzc[sum]; i++)
   {
    if(( lcmb[sumAsci->cindx[sum][i]].name != (char *)NULL ) && (strlen(lcmb[sumAsci->cindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(lcmb[sumAsci->cindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->cindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, lcmb[sumAsci->cindx[sum][i]].name, sumAsci->cindx[sum][i]);
      */
    }
   }    
  }
  else 
  {
    if(printFlag) printf(" WARNING lcmb:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->c; i++) if( lcmb[i].name != (char *)NULL )
      if((compare( lcmb[i].name, name, length)==length) && (sword( lcmb[i].name, buffer)==length))
        return(i);
  }

  /* if not found look for free cells */
  if (delLcmbFlag)
  for (i=0; i<anzGeo->c; i++)
  {
    /* do we have a "deleted" lcmb for use?  */
    if( lcmb[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delLcmbFlag=0;
  return(pfree);
}


/* returns surf-Index if known, or -(Index+10) of a deleted surf or if none is deleted -1  */
int getSurfNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->s) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  /*
  printf ("in getSurfNr anzs[%d]: %d\n", sum, sumAsci->anzs[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal Surf name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sums)
  {
   for (i=0; i<sumAsci->anzs[sum]; i++)
   {
    if(( surf[sumAsci->sindx[sum][i]].name != (char *)NULL ) && (strlen(surf[sumAsci->sindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(surf[sumAsci->sindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->sindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, surf[sumAsci->sindx[sum][i]].name, sumAsci->sindx[sum][i]);
      */
    }
   } 
  }
  else
  {
    if(printFlag) printf(" WARNING surf:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL )
    {
      if((compare( surf[i].name, name, length)==length) && (sword( surf[i].name, buffer)==length)) return(i);
    }
  }

  /* if not found look for free cells */
  if (delSurfFlag)
  for (i=0; i<anzGeo->s; i++)
  {
    /* do we have a "deleted" surf for use?  */
    if( surf[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delSurfFlag=0;
  return(pfree);
}

/* returns body-Index if known, or -(Index+10) of a deleted body or if none is deleted -1  */
int getBodyNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->b) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  /*
  printf ("in getBodyNr anzb[%d]: %d\n", sum, sumAsci->anzb[sum]);
  */
  if(sum<0)
  {
    printf ("ERROR: Illegal Body name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumb)
  {
   for (i=0; i<sumAsci->anzb[sum]; i++)
   {
    if(( body[sumAsci->bindx[sum][i]].name != (char *)NULL ) && (strlen(body[sumAsci->bindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(body[sumAsci->bindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->bindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, body[sumAsci->bindx[sum][i]].name, sumAsci->bindx[sum][i]);
      */
    }
   }   
  }
  else 
  {
    if(printFlag) printf(" WARNING body:%s not found in hash-table\n", name);
    for (i=0; i<anzGeo->b; i++) if( body[i].name != (char *)NULL )
      if((compare( body[i].name, name, length)==length) && (sword( body[i].name, buffer)==length))
        return(i);
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delBodyFlag)
  for (i=0; i<anzGeo->b; i++)
  {
    /* do we have a "deleted" body for use?  */
    if( body[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delBodyFlag=0;
  return(pfree);
}


/* returns Nurs-Index if known, or -(Index+10) of a deleted body or if none is deleted -1  */
int getNursNr(char *name)
{
  static int  i, n, length, pfree, sum;

  if(!anzGeo->nurs) return(-1);
  if(compareStrings(name, "BLEND")>0) return(-1);

  i=length=sum=0;
  pfree=-1;

  while(name[length]!='\0') {  sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Nurs name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }

  if(sum<=sumAsci->max_sumS)
  {
   for (i=0; i<sumAsci->anzS[sum]; i++)
   {
    if(( nurbs[sumAsci->Sindx[sum][i]].name != (char *)NULL ) && (strlen(nurbs[sumAsci->Sindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(nurbs[sumAsci->Sindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->Sindx[sum][i]);
	}
      }
      //printf(" name:%s nam:%s indx:%d\n", name, nurbs[sumAsci->Sindx[sum][i]].name, sumAsci->Sindx[sum][i]);
    }
   }
  }
  else 
  {
   if(printFlag)
     printf("WARNING NURS:%s not found in hash-table\n", name);
   for (i=0; i<anzGeo->nurs; i++)
   {
     // printf(" nurbsname:%d %d\n", i, nurbs[i].name);
    if( nurbs[i].name != (char *)NULL )
    {
      //printf("nurs:%s length:%d name:%s length:%d\n", nurbs[i].name, strlen(nurbs[i].name), name, length);
      if((compare( nurbs[i].name, name, length)==length) && (sword( nurbs[i].name, buffer)==length))
        return(i);
    }
   }
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delNursFlag)
  for (i=0; i<anzGeo->nurs; i++)
  {
    /* do we have a "deleted" nurbs for use?  */
    if( nurbs[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delNursFlag=0;
  return(pfree);
}

/* returns nurl-Index if known, or -(Index+10) of a deleted nurl or if none is deleted -1  */
int getNurlNr(char *name)
{
  static int i, length, pfree;

  if(!anzGeo->nurl) return(-1);

  i=0;
  pfree=-1;
  length=sword( name, buffer );
  if(!length) return(-1);

  for (i=0; i<anzGeo->nurl; i++)
  {
    if( ( compare( nurbl[i].name, name, length)==length) && ( sword( nurbl[i].name, buffer)==length) )
      return(i);
    if( nurbl[i].name == (char *)NULL )
      pfree=-i-10;
  }
  return(pfree);
}



/* returns set-Index if known, or -(Index+10) of a deleted set or if none is deleted -1  */
int getSetNr(char *name)
{
  int  i, n, length, pfree, sum;

  if(!anz->sets) return(-1);
  if(name== (char *)NULL) return(-1);

  i=length=sum=0;
  pfree=-1;

  while((name[length]!='\0')&&(length<MAX_LINE_LENGTH)) { sum+=name[length++]*(++i); }
  if(!length) return(-1);
  
  if(sum<0)
  {
    printf ("ERROR: Illegal Set name:|%s| sum-ascii:%d\n", name, sum);
    strcpy(name,"0000");
    return(-1); 
  }
  if(sum<=sumAsci->max_sumse)
  {
   for (i=0; i<sumAsci->anzse[sum]; i++)
   {
    if(( set[sumAsci->seindx[sum][i]].name != (char *)NULL ) && (strlen(set[sumAsci->seindx[sum][i]].name) == length))
    { 
      n=length-1;
      while(set[sumAsci->seindx[sum][i]].name[n]==name[n])
      {
        if(!n--)
        {
          return(sumAsci->seindx[sum][i]);
	}
      }
      /*
      printf(" name:%s nam:%s indx:%d\n"
      , name, set[sumAsci->seindx[sum][i]].name, sumAsci->seindx[sum][i]);
      */
    }
   }    
  }

  if(printFlag) printf(" WARNING set:%s not found in hash-table\n", name);
  for (i=0; i<anz->sets; i++)
  {
    if (set[i].name!= (char *)NULL)
    {
      if ((compare( set[i].name, name, length)==length) && (sword( set[i].name, buffer)==length))
        return(i);
    }
  }

  /* set not found, check if the name could be an set-index */
  if (checkIfNumber(name)>0)
  {
    /* the user has specified the set-index */
    n=atoi(name);
    if(n<=anz->sets)
    {
      for (i=0; i<anz->sets; i++) if(set[i].index==n)
      {
        printf (" index %s points to %s\n", name, set[i].name);
        return (i);
      }
    }
  }

  /* if not found check all and look for free cells */
  /* check all is not active, should not be necessary */
  if (delSetFlag)
  for (i=0; i<anz->sets; i++)
  {
    /* do we have a "deleted" set for use?  */
    if( set[i].name == (char *)NULL ) 
    {
      pfree=-i-10;
      return(pfree);
    }
  }
  delSetFlag=0;
  return(pfree);
}




/* search an element in an sorted integer-array */
/* return -1 if not found or index if found */
int getIndex(int **ipnt, int n, int x0 )
{
  int i=0,ii,m,n1,n2;
  int *x;

  x=*ipnt;
  /* if x0 is lower than the first elem */
  if((n==0)||(x0<x[0])) return(-1);

  /* if x0 is higher than the last elem */
  else if(x0>x[n-1]) return(-1);

  else
  {
    /* search the intersection */
    n1=0;                              
    n2=n;                            
    for(ii=0; ii<n; ii++)
    {                     
      m=(n2+n1)/2;                      
      if(x0>= x[m] ) n1=m;              
      if(x0 < x[m] ) n2=m;              
      if((n2-n1) == 1) break;           
    }                                 
    i=n1;
#if TEST
    printf("i:%d x:%d x0:%d x++:%d\n", i,x[i],x0,x[i+1]); 
#endif

    if (x0!=x[i])
    {
      /* element not found */
      return(-1);
    }
  }

#if TEST
  for(ii=0; ii<n; ii++)
  {
    printf("i:%d x:%d \n", ii,x[ii]); 
  }
#endif
 
#if TEST
  printf("b:%d\n", *(*ipnt)); 
#endif
  return(n);
}



void delSet( char *setname)
{
  int i, j;
  int   setNr;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    if(printFlag) printf (" delSet: set:%s does not exist\n", setname);
    return;
  }
  /* suche abhaengige entities */
  if( set[setNr].type )
  {
    /* durchsuche alle linien   */
    for(i=0; i<anzGeo->l; i++)
    {
      if ((setNr==line[i].trk)&&(line[i].typ=='s')&&( line[i].name != (char *)NULL))
      {
        printf (" ERROR: trk removed, Line will be straight\n");
        line[i].trk=-1;
        line[i].typ=' ';
      }
    }
  }
  /* durchsuche alle uebrigen sets und entferne den set  */
  for(i=0; i<anz->sets; i++)
  {
    for(j=0; j<set[i].anz_se; j++)
      if (setNr==set[i].set[j])
      {
        setr( i, "r", setNr);
      }
  }
  /* durchsuche alle psets und entferne den set   */
  j=0; for(i=0; i<anzGeo->psets; i++)
  {
    if (setNr!=pset[i].nr)
    {
      if(i!=j) {
        pset[j].nr=pset[i].nr;
        strcpy(pset[j].type,pset[i].type);
        pset[j].col=pset[i].col;
      }
      j++;
    }
  }
  anzGeo->psets=j;

  /* der set wird wieder frei gegeben */
  delSetFlag=1;
  if(set[setNr].anz_elf)
    for(i=0; i<set[setNr].anz_elf; i++)
      if(set[setNr].elf[i].n) free(set[setNr].elf[i].v);  
    free(set[setNr].name);
    free(set[setNr].valu);
    free(set[setNr].node);
    free(set[setNr].elem);
    free(set[setNr].face);
    free(set[setNr].elf);
    free(set[setNr].pnt);
    free(set[setNr].line);
    free(set[setNr].lcmb);
    free(set[setNr].surf);
    free(set[setNr].body);
    free(set[setNr].nurl);
    free(set[setNr].nurs);
    free(set[setNr].set);
    free(set[setNr].shp);
    free(set[setNr].eparm);
    set[setNr].name=(char *)NULL;
    set[setNr].valu=(int *)NULL;
    set[setNr].node=(int *)NULL;
    set[setNr].elem=(int *)NULL;
    set[setNr].face=(int *)NULL;
    set[setNr].elf=(Elfaces *)NULL;
    set[setNr].pnt= (int *)NULL;
    set[setNr].line=(int *)NULL;
    set[setNr].lcmb=(int *)NULL;
    set[setNr].surf=(int *)NULL;
    set[setNr].body=(int *)NULL;
    set[setNr].nurl=(int *)NULL;
    set[setNr].nurs=(int *)NULL;
    set[setNr].set=(int *)NULL;
    set[setNr].shp=(int *)NULL;
    set[setNr].eparm=(char *)NULL;
    set[setNr].material = -1;
    set[setNr].flag = 'c';
    set[setNr].type = 0;
    set[setNr].anz_v = 0;
    set[setNr].anz_n = 0;
    set[setNr].anz_e = 0;
    set[setNr].anz_f = 0;
    set[setNr].anz_elf = 0;
    set[setNr].anz_p = 0;
    set[setNr].anz_l = 0;
    set[setNr].anz_c = 0;
    set[setNr].anz_s = 0;
    set[setNr].anz_b = 0;
    set[setNr].anz_nurl = 0;
    set[setNr].anz_nurs = 0;
    set[setNr].anz_se = 0;
    set[setNr].anz_sh = 0;
}

/*------------------------------------------------------------------*/
/* einen set als offen markieren                                    */
/*------------------------------------------------------------------*/

int seto( char *setname )
{
  int   i,setNr;

  operateAlias( setname, "se" );

  setNr=getSetNr(setname);

  if (strlen(setname)<=0)
  {
    /* list all open sets */
    for (i=0; i<anz->sets; i++)
    {
      if( set[i].name != (char *)NULL )
      {
        if ((!set[i].type)&&(set[i].flag=='o'))
          printf ("%s stat:%c n:%d e:%d f:%d p:%d l:%d s:%d b:%d L:%d S:%d se:%d sh:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh);
      }
    }
    return(-1);
  }

  if (setNr==-1)       /* open a new set */
  {
    if ((set = (Sets *)realloc( (Sets *)set, (anz->sets+1)*sizeof(Sets)) ) == NULL )
    {
      printf(" ERROR: realloc failure in seto, set:%s not installed\n\n", setname);
      return(-1);
    }
    setNr= anz->sets;
    anz->sets++;

    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    if(printFlag) printf (" create and open set:%s\n", set[setNr].name);
    set[setNr].flag='o';
    set[setNr].type=0;
    set[setNr].etyp=0;
    set[setNr].eattr=0;
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].index = -1;
    set[setNr].material = -1;
    set[setNr].anz_v = 0;
    set[setNr].anz_n = 0;
    set[setNr].anz_e = 0;
    set[setNr].anz_f = 0;
    set[setNr].anz_elf = 0;
    set[setNr].anz_p = 0;
    set[setNr].anz_l = 0;
    set[setNr].anz_c = 0;
    set[setNr].anz_s = 0;
    set[setNr].anz_b = 0;
    set[setNr].anz_nurl = 0;
    set[setNr].anz_nurs = 0;
    set[setNr].anz_se = 0;
    set[setNr].anz_sh = 0;
    set[setNr].eparm=(char *)NULL;
  }
  else if (setNr<-1)    /* replace a deleted set */
  {
    setNr=-(setNr+10);
    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    set[setNr].flag='o';
    set[setNr].type=0;
    if(printFlag) printf (" open set:%s\n", set[setNr].name);
  }
  else
  {
    if (set[setNr].flag=='o')
    {
      printf (" set:%s is already open\n", set[setNr].name);
      return(setNr);
    }
    else
    {
      if(printFlag) printf (" open set:%s\n", set[setNr].name);
      set[setNr].flag='o';
    }
  }

  /* add the set to openSets */
  openSets->nr++;
  if( (openSets->set = (int *)realloc( (int *)openSets->set, openSets->nr*sizeof(int))) == NULL )
  { printf(" ERROR: realloc failed in seto\n\n"); return(-1); }
  openSets->set[ openSets->nr-1 ] = setNr;
  if(printFlag) printf("seto: openSets->nr:%d openSets->set:%d set:%s\n", openSets->nr, openSets->set[openSets->nr-1],set[setNr].name);
  return(setNr);
}


/*------------------------------------------------------------------*/
/* einen set schliessen                                             */
/*------------------------------------------------------------------*/

void setc( char *setname )
{
  int   setNr, i, n, flag=0;
  static int *puf=NULL;

  operateAlias( setname, "se" );
  if (strlen(setname)>0) setNr=getSetNr(setname);
  else if(openSets->nr>0) setNr=openSets->set[openSets->nr-1];
  else { printf("no open set\n"); return; }

  if (setNr<0)
  {
    printf (" setc: set:%s does not exist\n", setname);
    return;
  }
  else
  {
    if (set[setNr].flag=='c')
    {
      printf (" set:%s is already closed\n", set[setNr].name);
    }
    else
    {
      if(printFlag) printf (" close set:%s\n", set[setNr].name);
      set[setNr].flag='c';

      /* remove the set from openSets */
      /* puffer allocieren  */
      if ( (puf = (int *)malloc( (openSets->nr+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setc\n\n");

      n=0;
      for ( i=0; i<openSets->nr; i++)
      {
        if ( openSets->set[i]!=setNr )
        {
          puf[n] = openSets->set[i];
          n++;
        }
        else
          flag=1;
      }
 
      if (flag)
      {
        openSets->nr--;
        if ((openSets->set = (int *)realloc( (int *)openSets->set, (openSets->nr+1)*sizeof(int)) ) == NULL)
        { printf(" ERROR: realloc failed in setc\n\n"); free(puf); return; }

        for ( i=0; i<openSets->nr; i++)
        {
          openSets->set[i] = puf[i];
        }
      }
      free(puf);
      if(printFlag) printf("setc: openSets->nr:%d openSets->set:%d set:%s\n", openSets->nr, setNr, set[setNr].name); 
    }
  }
}


/*------------------------------------------------------------------*/
/* entity einem Set zuordnen                                        */
/*------------------------------------------------------------------*/


int hashSet( SumAsci *sumAsci, char *name, int nr)
{
  int i=0,j=0, n;
  int sum=0; 

  while(name[i]!='\0') { sum+=name[i]*(++j); i++; }

  /* check if sum is higher as the allocated value */
  /* if not look for a free entry */
  if(sum>sumAsci->max_sumse)
  {
    if ((sumAsci->anzse=(int *)realloc( (int *)sumAsci->anzse, (sum+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }
    if ((sumAsci->seindx=(int **)realloc( (int **)sumAsci->seindx, (sum+1)*sizeof(int *)) ) == NULL )
    { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }
    for(i=sumAsci->max_sumse+1; i<=sum; i++) { sumAsci->anzse[i]=0; sumAsci->seindx[i]=NULL; }
    sumAsci->max_sumse=sum;
  }
  else
  {
    if (delSetFlag)
    for (i=0; i<sumAsci->anzse[sum]; i++) 
    {
      n=sumAsci->seindx[sum][i];
      if( set[n].name == (char *)NULL )
      {
        /* already existing space to fill */
        sumAsci->seindx[sum][i]=nr;
        return(sum);
      }
    }
  }

  /* alloc of a new entry in the hash table */
  if ((sumAsci->seindx[sum] =(int *)realloc( (int *)sumAsci->seindx[sum], (sumAsci->anzse[sum]+1)*sizeof(int)) ) == NULL )
  { printf("\n\nERROR: realloc failure in hashSet(), set:%s not included\n\n", name); return(-1); }

  sumAsci->seindx[sum][sumAsci->anzse[sum]] = nr;
  sumAsci->anzse[sum]++;
  return(sum);
}



int pre_seta( char *string, char *type, char *name)
{
  int i, setNr;
  int n=0;
  int number;
  char setname[MAX_LINE_LENGTH]; /* string is not changeable. Therefore a new char is necessary */

  /* remove blanks and other illegal chars*/
  for(i=0;i<strlen(string); i++) if(string[i]>(char)0x20) { setname[n]=string[i]; n++; }
  if(!n) return(-1);
  setname[n]='\0';

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  number=0;

  if (setNr==-1)
  {
    if ((set = (Sets *)realloc( (Sets *)set, (anz->sets+2)*sizeof(Sets)) ) == NULL )
    {
      printf(" ERROR: realloc failure in pre_seta, set:%s not installed\n\n", setname);
      return(-1);
    }
    setNr= anz->sets;
    anz->sets++;

    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    hashSet( sumAsci, setname, setNr );
    if(printFlag) printf (" create set:%s\n", set[setNr].name);
    set[setNr].flag='c';
    if ( type[1] == 's' ) set[setNr].type=1;
    else                  set[setNr].type=0;
    set[setNr].etyp=0;
    set[setNr].eattr=0;
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].index = -1;
    set[setNr].material = -1;
    set[setNr].anz_v = 0;
    set[setNr].anz_n = 0;
    set[setNr].anz_e = 0;
    set[setNr].anz_f = 0;
    set[setNr].anz_elf = 0;
    set[setNr].anz_p = 0;
    set[setNr].anz_l = 0;
    set[setNr].anz_c = 0;
    set[setNr].anz_s = 0;
    set[setNr].anz_b = 0;
    set[setNr].anz_nurl = 0;
    set[setNr].anz_nurs = 0;
    set[setNr].anz_se = 0;
    set[setNr].anz_sh = 0;
    set[setNr].eparm=(char *)NULL;
  }
  else if (setNr<-1)    /* replace a deleted set */
  {
    setNr=-(setNr+10);
    i=strlen(setname);
    if((set[setNr].name= (char *)malloc((i+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    strcpy( set[setNr].name, setname);
    if ( type[1] == 's' ) set[setNr].type=1;
    else                  set[setNr].type=0;
    hashSet( sumAsci, setname, setNr );
    if((set[setNr].valu= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].node= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elem= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].face= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].elf= (Elfaces *)malloc(sizeof(Elfaces))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].pnt= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].line= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].lcmb= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].surf= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].body= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurl= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].nurs= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].set= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    if((set[setNr].shp= (int *)malloc(sizeof(int))) == NULL )
    { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    set[setNr].index = -1;
    set[setNr].material = -1;
    set[setNr].anz_v = 0;
    set[setNr].anz_n = 0;
    set[setNr].anz_e = 0;
    set[setNr].anz_f = 0;
    set[setNr].anz_elf = 0;
    set[setNr].anz_p = 0;
    set[setNr].anz_l = 0;
    set[setNr].anz_c = 0;
    set[setNr].anz_s = 0;
    set[setNr].anz_b = 0;
    set[setNr].anz_nurl = 0;
    set[setNr].anz_nurs = 0;
    set[setNr].anz_se = 0;
    set[setNr].anz_sh = 0;
    set[setNr].eparm=(char *)NULL;
  }

  /* determine the index of the entity */
  if (( type[0] == 's' )&&( type[1] == 'e' ))  number=getSetNr(name);
  else if (( type[0] == 's' )&&( type[1] == 'h' ))  number=getShapeNr(name);
  else if ( type[0] == 'n' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'e' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'f' )                   if(checkIfNumber(name)) number=atoi(name); else return(-1);
  else if ( type[0] == 'v' )                   number=getValuNr(name);
  else if ( type[0] == 'p' )                   number=getPntNr(name);
  else if ( type[0] == 'l' )                   number=getLineNr(name);
  else if ( type[0] == 'c' )                   number=getLcmbNr(name);
  else if ( type[0] == 's' )                   number=getSurfNr(name);
  else if ( type[0] == 'b' )                   number=getBodyNr(name);
  else if ( type[0] == 'L' )                   number=getNurlNr(name);
  else if ( type[0] == 'S' )                   number=getNursNr(name);
  else if ( type[0] == 'r' )                   number=getSetNr(name);
  else if ( type[0] == 'i' ) 
  {
    if(printFlag) printf (" set initialized\n");
    return(setNr);
  }
  else
  {
    errMsg ("ERROR: in seta type:%s not recognized\n", type);
    return(-1);
  }
  if(number<0)
  {
    if(printFlag) errMsg ("WARNING: in seta entity:%s not known\n", name);
    return(-1);
  }

  /* add to the set */
  if( seta(setNr, type, number)<0 )
  {
    errMsg ("ERROR: seta failed\n", name);
    return(-1);
  }
  else
    return(setNr);
}



int rnam( int setNr, char *new_name)
{
  if((set[setNr].name= (char *)realloc((char *)set[setNr].name, (strlen(new_name)+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed\n\n" ); return(-1); }
  strcpy(set[setNr].name, new_name );
  hashSet( sumAsci,set[setNr].name, setNr );
  return(setNr);
}



int seta( int setNr, char *type, int number)
{
  int n,k;
  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(-1);
  }
  if( number<0)
  {
    errMsg(" ERROR in seta: A negative entity-index:%d was used in set %s\n", number,set[setNr].name);
    return(-1);
  }

  /* check if item is known and if its already member of the set */
  if ( type[0] == 'r' )
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_se= iinsert(&set[setNr].set, set[setNr].anz_se, number);
    set[number].anz_se= iinsert(&set[number].set, set[number].anz_se, setNr);
  }
  else if ( type[0] == 'v' )
  {
    set[setNr].anz_v=iinsert(&set[setNr].valu, set[setNr].anz_v, number);
  }
  else if (( type[0] == 's' )&&( type[1] == 'e' ))
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_se= iinsert(&set[setNr].set, set[setNr].anz_se, number);
  }
  else if (( type[0] == 'h' )||(( type[0] == 's' )&&( type[1] == 'h' )))
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: shape:%d in set %s does not exist\n", number,set[setNr].name ); return(-1); }
    set[setNr].anz_sh= iinsert(&set[setNr].shp, set[setNr].anz_sh, number);
  }
  else if ( type[0] == 'n' )
  {
    if ((number<anz->nmin)|| (number>anz->nmax)||(node[number].indx<0) || (number!=node[node[number].indx].nr)||(node[number].pflag!=0))    
    {  printf(" ERROR in seta: node:%d does not exist and can not be placed in set %s, min:%d max:%d\n", number,set[setNr].name, anz->nmin, anz->nmax); return(-1); }

    if(set[setNr].type==1) /* seqa */
    {
      for (n=0; n<set[setNr].anz_n; n++)
      {
        if( set[setNr].node[n] == number )
        { if(printFlag) printf(" ERROR in seta: node already stored in set %s\n",set[setNr].name); return(-1); }
      }
      set[setNr].anz_n++;
      if( (set[setNr].node = (int *)realloc( (int *)set[setNr].node, set[setNr].anz_n*sizeof(int))) == NULL )
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); return(-1); }
      else
      {
        if(printFlag) printf(" set[%d]:%s reallocated and node %d added\n"
            , setNr, set[setNr].name, number );
      }
      set[setNr].node[ (set[setNr].anz_n-1) ] = number;
    }
    else
      set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, number);
  }
  else if ( type[0] == 'e' )
  {
    if ((number<anz->emin)|| (number>anz->emax) || (e_enqire[number].type==0))
      { printf(" ERROR in seta: set %s, elem:%d does not exist, min:%d max:%d\n", set[setNr].name, number, anz->emin, anz->emax ); return(-1); }
    set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, number);
  }
  else if ( type[0] == 'f' )
  {
    set[setNr].anz_f=iinsert(&set[setNr].face, set[setNr].anz_f, number);
  }
  else if ( type[0] == 'p' )
  {
    if (number<0)
    { if(printFlag) printf(" ERROR in seta: set %s, pntNr:%d does not exist\n", set[setNr].name, number ); return(-1); }

    if(set[setNr].type==1) /* seqa */
    {
      if (number<0)
      { if(printFlag) printf(" ERROR in seta: set %s, pntNr:%d does not exist\n", set[setNr].name, number ); return(-1); }
      for (n=0; n<set[setNr].anz_p; n++)
      {
        if( set[setNr].pnt[n] == number )
        { if(printFlag) printf(" ERROR in seta: pnt already stored in set %s\n", set[setNr].name); return(-1); }
      }
      set[setNr].anz_p++;
      if( (set[setNr].pnt = (int *)realloc( (int *)set[setNr].pnt, set[setNr].anz_p*sizeof(int))) == NULL )
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); return(-1); }
      else
      {
        if(printFlag) printf(" set[%d]:%s reallocated and pnt %s added\n"
            , setNr, set[setNr].name, point[number].name );
      }
      set[setNr].pnt[ (set[setNr].anz_p-1) ] = number;
    }
    else
      set[setNr].anz_p=iinsert(&set[setNr].pnt, set[setNr].anz_p, number);

    if (point[number].nn>0)
    {
      for (k=0; k<point[number].nn; k++)
      {
        set[setNr].anz_n=iinsert(&set[setNr].node, set[setNr].anz_n, point[number].nod[k] );
      }
    }
  }
  else if ( type[0] == 'l' )
  {
    if (number<0)
    { if(printFlag) printf(" lineNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_l= iinsert(&set[setNr].line, set[setNr].anz_l, number);
    if( line[number].nn >0 )
    {
      for (k=0; k<line[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, line[number].nod[k] );
      }
    }
    if (line[number].ne>0)
    {
      for (k=0; k<line[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, line[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'c' )
  {
    if (number<0)
    { if(printFlag) printf(" lcmbNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_c= iinsert(&set[setNr].lcmb, set[setNr].anz_c, number);
  }
  else if ( type[0] == 's' )
  {
    if (number<0)
    { if(printFlag) printf(" surfNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_s= iinsert(&set[setNr].surf, set[setNr].anz_s, number);
    if (surf[number].nn>0)
    {
      for (k=0; k<surf[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n, surf[number].nod[k] );
      }
    }
    if (surf[number].ne>0)
    {
      for (k=0; k<surf[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e, surf[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'b' )
  { 
    if (number<0)
    { if(printFlag) printf(" bodyNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_b= iinsert(&set[setNr].body, set[setNr].anz_b, number);

    if (body[number].nn>0)
    {
      for (k=0; k<body[number].nn; k++)
      {
        set[setNr].anz_n= iinsert(&set[setNr].node, set[setNr].anz_n,body[number].nod[k] );
      }
    }
    if (body[number].ne>0)
    {
      for (k=0; k<body[number].ne; k++)
      {
        set[setNr].anz_e=iinsert(&set[setNr].elem, set[setNr].anz_e,body[number].elem[k] );
      }
    }
  }
  else if ( type[0] == 'L' )
  {
    if (number<0)
    { if(printFlag) printf(" NurlNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_nurl= iinsert(&set[setNr].nurl, set[setNr].anz_nurl, number);
  }
  else if ( type[0] == 'S' )
  {
    if (number<0)
    { if(printFlag) printf(" NursNr:%d does not exist\n", number ); return(-1); }
    set[setNr].anz_nurs= iinsert(&set[setNr].nurs, set[setNr].anz_nurs, number);
  }
  else if ( type[0] == 'j' )
  {
    if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf+1)*sizeof(Elfaces))) == NULL )
    { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
    if(number)
    {
      set[setNr].elf[set[setNr].anz_elf].n=number;
      if((set[setNr].elf[set[setNr].anz_elf].v= (float *)calloc(number, sizeof(float))) == NULL )
      { printf("ERROR: malloc failed in seta()\n\n" ); return(-1); }
    }
    else set[setNr].elf[set[setNr].anz_elf].n=0;
    set[setNr].anz_elf++;
    return(set[setNr].anz_elf-1);
  }
  else
  {
    errMsg ("WARNING: in seta type:%s not recognized\n", type);
    return(-1);
  }
  return(1);
}


int sete( int setNr, char *type, char *mode)
{
  int i=0,s;
  int failFlag=0;

  /* check if all entities of setNr are also members in another sets */
  if ((type[0]=='s')&&(type[1]=='e'))
  {
    for(s=0; s<anz->sets; s++)
    {
      failFlag=0;
      if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
      {
        if(set[setNr].anz_n> set[s].anz_n) continue;
        if(set[setNr].anz_e> set[s].anz_e) continue;
        if(set[setNr].anz_f> set[s].anz_f) continue;
        if(set[setNr].anz_p> set[s].anz_p) continue;
        if(set[setNr].anz_l> set[s].anz_l) continue;
        if(set[setNr].anz_c> set[s].anz_c) continue;
        if(set[setNr].anz_s> set[s].anz_s) continue;
        if(set[setNr].anz_b> set[s].anz_b) continue;
        if(set[setNr].anz_se> set[s].anz_se) continue;
        if(set[setNr].anz_sh> set[s].anz_sh) continue;
        if(set[setNr].anz_nurl> set[s].anz_nurl) continue;
        if(set[setNr].anz_nurs> set[s].anz_nurs) continue;
        if((s!=setNr)&&(set[s].name!=NULL))
	{
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(0>ifind(&set[s].node, set[s].anz_n, set[setNr].node[i] )) break;
          }
          if(i!=set[setNr].anz_n) failFlag=1;
          for (i=0; i<set[setNr].anz_e; i++)
          {
            if(0>ifind(&set[s].elem, set[s].anz_e, set[setNr].elem[i] )) break;
          }
          if(i!=set[setNr].anz_e) failFlag=1;
          for (i=0; i<set[setNr].anz_f; i++)
          {
            if(0>ifind(&set[s].face, set[s].anz_f, set[setNr].face[i] )) break;
          }
          if(i!=set[setNr].anz_f) failFlag=1;
          for (i=0; i<set[setNr].anz_p; i++)
          {
            if(0>ifind(&set[s].pnt, set[s].anz_p, set[setNr].pnt[i] )) break;
          }
          if(i!=set[setNr].anz_p) failFlag=1;
          for (i=0; i<set[setNr].anz_l; i++)
          {
            if(0>ifind(&set[s].line, set[s].anz_l, set[setNr].line[i] )) break;
          }
          if(i!=set[setNr].anz_l) failFlag=1;
          for (i=0; i<set[setNr].anz_c; i++)
          {
            if(0>ifind(&set[s].lcmb, set[s].anz_c, set[setNr].lcmb[i] )) break;
          }
          if(i!=set[setNr].anz_c) failFlag=1;
          for (i=0; i<set[setNr].anz_s; i++)
          {
            if(0>ifind(&set[s].surf, set[s].anz_s, set[setNr].surf[i] )) break;
          }
          if(i!=set[setNr].anz_s) failFlag=1;
          for (i=0; i<set[setNr].anz_b; i++)
          {
            if(0>ifind(&set[s].body, set[s].anz_b, set[setNr].body[i] )) break;
          }
          if(i!=set[setNr].anz_b) failFlag=1;
          for (i=0; i<set[setNr].anz_se; i++)
          {
            if(0>ifind(&set[s].set, set[s].anz_se, set[setNr].set[i] )) break;
          }
          if(i!=set[setNr].anz_se) failFlag=1;
          for (i=0; i<set[setNr].anz_sh; i++)
          {
            if(0>ifind(&set[s].shp, set[s].anz_sh, set[setNr].shp[i] )) break;
          }
          if(i!=set[setNr].anz_sh ) failFlag=1;
          for (i=0; i<set[setNr].anz_nurl; i++)
          {
            if(0>ifind(&set[s].nurl, set[s].anz_nurl, set[setNr].nurl[i] )) break;
          }
          if(i!=set[setNr].anz_nurl) failFlag=1;
          for (i=0; i<set[setNr].anz_nurs; i++)
          {
            if(0>ifind(&set[s].nurs, set[s].anz_nurs, set[setNr].nurs[i] )) break;
          }
          if(i!=set[setNr].anz_nurs) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All entities from set:%s are also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(
                (set[setNr].anz_n==set[s].anz_n)&&
                (set[setNr].anz_e==set[s].anz_e)&&
                (set[setNr].anz_f==set[s].anz_f)&&
                (set[setNr].anz_p==set[s].anz_p)&&
                (set[setNr].anz_l==set[s].anz_l)&&
                (set[setNr].anz_c==set[s].anz_c)&&
                (set[setNr].anz_s==set[s].anz_s)&&
                (set[setNr].anz_b==set[s].anz_b)&&
                (set[setNr].anz_se==set[s].anz_se)&&
                (set[setNr].anz_sh==set[s].anz_sh)&&
                (set[setNr].anz_nurl==set[s].anz_nurl)&&
                (set[setNr].anz_nurs==set[s].anz_nurs) )
                  printf(" set:%s and %s share identical entities\n", set[setNr].name, set[s].name);
	    }
	  }
	}
      }
      else /* max */
      {
        if(set[setNr].anz_n< set[s].anz_n) continue;
        if(set[setNr].anz_e< set[s].anz_e) continue;
        if(set[setNr].anz_f< set[s].anz_f) continue;
        if(set[setNr].anz_p< set[s].anz_p) continue;
        if(set[setNr].anz_l< set[s].anz_l) continue;
        if(set[setNr].anz_c< set[s].anz_c) continue;
        if(set[setNr].anz_s< set[s].anz_s) continue;
        if(set[setNr].anz_b< set[s].anz_b) continue;
        if(set[setNr].anz_se< set[s].anz_se) continue;
        if(set[setNr].anz_sh< set[s].anz_sh) continue;
        if(set[setNr].anz_nurl< set[s].anz_nurl) continue;
        if(set[setNr].anz_nurs< set[s].anz_nurs) continue;
        if((s!=setNr)&&(set[s].name!=NULL))
	{
          for (i=0; i<set[s].anz_n; i++)
          {
            if(0>ifind(&set[setNr].node, set[setNr].anz_n, set[s].node[i] )) break;
          }
          if(i!=set[setNr].anz_n) failFlag=1;
          for (i=0; i<set[s].anz_e; i++)
          {
            if(0>ifind(&set[setNr].elem, set[setNr].anz_e, set[s].elem[i] )) break;
          }
          if(i!=set[s].anz_e) failFlag=1;
          for (i=0; i<set[s].anz_f; i++)
          {
            if(0>ifind(&set[setNr].face, set[setNr].anz_f, set[s].face[i] )) break;
          }
          if(i!=set[s].anz_f) failFlag=1;
          for (i=0; i<set[s].anz_p; i++)
          {
            if(0>ifind(&set[setNr].pnt, set[setNr].anz_p, set[s].pnt[i] )) break;
          }
          if(i!=set[s].anz_p) failFlag=1;
          for (i=0; i<set[s].anz_l; i++)
          {
            if(0>ifind(&set[setNr].line, set[setNr].anz_l, set[s].line[i] )) break;
          }
          if(i!=set[s].anz_l) failFlag=1;
          for (i=0; i<set[s].anz_c; i++)
          {
            if(0>ifind(&set[setNr].lcmb, set[setNr].anz_c, set[s].lcmb[i] )) break;
          }
          if(i!=set[s].anz_c) failFlag=1;
          for (i=0; i<set[s].anz_s; i++)
          {
            if(0>ifind(&set[setNr].surf, set[setNr].anz_s, set[s].surf[i] )) break;
          }
          if(i!=set[s].anz_s) failFlag=1;
          for (i=0; i<set[s].anz_b; i++)
          {
            if(0>ifind(&set[setNr].body, set[setNr].anz_b, set[s].body[i] )) break;
          }
          if(i!=set[s].anz_b) failFlag=1;
          for (i=0; i<set[s].anz_se; i++)
          {
            if(0>ifind(&set[setNr].set, set[setNr].anz_se, set[s].set[i] )) break;
          }
          if(i!=set[s].anz_se) failFlag=1;
          for (i=0; i<set[s].anz_sh; i++)
          {
            if(0>ifind(&set[setNr].shp, set[setNr].anz_sh, set[s].shp[i] )) break;
          }
          if(i!=set[s].anz_sh ) failFlag=1;
          for (i=0; i<set[s].anz_nurl; i++)
          {
            if(0>ifind(&set[setNr].nurl, set[setNr].anz_nurl, set[s].nurl[i] )) break;
          }
          if(i!=set[s].anz_nurl) failFlag=1;
          for (i=0; i<set[s].anz_nurs; i++)
          {
            if(0>ifind(&set[setNr].nurs, set[setNr].anz_nurs, set[s].nurs[i] )) break;
          }
          if(i!=set[s].anz_nurs) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All entities from set:%s are also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }
    }
    return(1);
  }

  for(s=0; s<anz->sets; s++)
  {
    if((s!=setNr)&&(set[s].name!=NULL))
    {
      if ((type[0]=='s')&&(type[1]=='h'))
      {
        if(!set[setNr].anz_sh) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_sh> set[s].anz_sh) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_sh; i++)
          {
            if(0>ifind(&set[s].shp, set[s].anz_sh, set[setNr].shp[i] )) break;
          }
          if(i!=set[setNr].anz_sh ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All shapes from set:%s are also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_n==set[s].anz_n) printf(" set:%s and %s share identical shapes\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_sh< set[s].anz_sh) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_sh; i++)
          {
            if(0>ifind(&set[setNr].shp, set[setNr].anz_sh, set[s].shp[i] )) break;
          }
          if(i!=set[s].anz_sh ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All shapes from set:%s are also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='n')
      {
        if(!set[setNr].anz_n) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_n> set[s].anz_n) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_n; i++)
          {
            if(0>ifind(&set[s].node, set[s].anz_n, set[setNr].node[i] )) break;
          }
          if(i!=set[setNr].anz_n ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All nodes from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_n==set[s].anz_n) printf(" set:%s and %s share identical nodes\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_n< set[s].anz_n) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_n; i++)
          {
            if(0>ifind(&set[setNr].node, set[setNr].anz_n, set[s].node[i] )) break;
          }
          if(i!=set[s].anz_n ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nodes from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='e')
      {
        if(!set[setNr].anz_e) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_e> set[s].anz_e) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_e; i++)
          {
            if(0>ifind(&set[s].elem, set[s].anz_e, set[setNr].elem[i] )) break;
          }
          if(i!=set[setNr].anz_e ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All elems from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more elems as setNr */
	    {
              if(set[setNr].anz_e==set[s].anz_e) printf(" set:%s and %s share identical elems\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_e< set[s].anz_e) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_e; i++)
          {
            if(0>ifind(&set[setNr].elem, set[setNr].anz_e, set[s].elem[i] )) break;
          }
          if(i!=set[s].anz_e ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All elems from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='p')
      {
        if(!set[setNr].anz_p) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_p> set[s].anz_p) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_p; i++)
          {
            if(0>ifind(&set[s].pnt , set[s].anz_p, set[setNr].pnt[i] )) break;
          }
          if(i!=set[setNr].anz_p ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All points from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_p==set[s].anz_p) printf(" set:%s and %s share identical points\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_p< set[s].anz_p) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_p; i++)
          {
            if(0>ifind(&set[setNr].pnt , set[setNr].anz_p, set[s].pnt[i] )) break;
          }
          if(i!=set[s].anz_p ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All points from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='c')
      {
        if(!set[setNr].anz_c) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_c> set[s].anz_c) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_c; i++)
          {
            if(0>ifind(&set[s].lcmb, set[s].anz_c, set[setNr].lcmb[i] )) break;
          }
          if(i!=set[setNr].anz_c ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All lcmbs from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_c==set[s].anz_c) printf(" set:%s and %s share identical lcmbs\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_c< set[s].anz_c) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_c; i++)
          {
            if(0>ifind(&set[setNr].lcmb, set[setNr].anz_c, set[s].lcmb[i] )) break;
          }
          if(i!=set[s].anz_c ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All lcmbs from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='l')
      {
        if(!set[setNr].anz_l) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_l> set[s].anz_l) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_l; i++)
          {
            if(0>ifind(&set[s].line, set[s].anz_l, set[setNr].line[i] )) break;
          }
          if(i!=set[setNr].anz_l ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All lines from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_l==set[s].anz_l) printf(" set:%s and %s share identical lines\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_l< set[s].anz_l) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_l; i++)
          {
            if(0>ifind(&set[setNr].line, set[setNr].anz_l, set[s].line[i] )) break;
          }
          if(i!=set[s].anz_l ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All lines from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='s')
      {
        if(!set[setNr].anz_s) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_s> set[s].anz_s) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_s; i++)
          {
            if(0>ifind(&set[s].surf, set[s].anz_s, set[setNr].surf[i] )) break;
          }
          if(i!=set[setNr].anz_s ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All surfs from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_s==set[s].anz_s) printf(" set:%s and %s share identical surfs\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_s< set[s].anz_s) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_s; i++)
          {
            if(0>ifind(&set[setNr].surf, set[setNr].anz_s, set[s].surf[i] )) break;
          }
          if(i!=set[s].anz_s ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All surfs from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='b')
      {
        if(!set[setNr].anz_b) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_b> set[s].anz_b) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_b; i++)
          {
            if(0>ifind(&set[s].body, set[s].anz_b, set[setNr].body[i] )) break;
          }
          if(i!=set[setNr].anz_b ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All bodies from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_b==set[s].anz_b) printf(" set:%s and %s share identical bodies\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_b< set[s].anz_b) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_b; i++)
          {
            if(0>ifind(&set[setNr].body, set[setNr].anz_b, set[s].body[i] )) break;
          }
          if(i!=set[s].anz_b ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All bodies from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='L')
      {
        if(!set[setNr].anz_nurl) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_nurl> set[s].anz_nurl) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_nurl; i++)
          {
            if(0>ifind(&set[s].nurl, set[s].anz_nurl, set[setNr].nurl[i] )) break;
          }
          if(i!=set[setNr].anz_nurl ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All nurls from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_nurl==set[s].anz_nurl) printf(" set:%s and %s share identical nurls\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_nurl< set[s].anz_nurl) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_nurl; i++)
          {
            if(0>ifind(&set[setNr].nurl, set[setNr].anz_nurl, set[s].nurl[i] )) break;
          }
          if(i!=set[s].anz_nurl ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nurls from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

      else if (type[0]=='S')
      {
        if(!set[setNr].anz_nurs) continue;
        if(( compare( mode, "min", 3)==3)||( compare( mode, "strict", 3)==3))
        {
          if(set[setNr].anz_nurs> set[s].anz_nurs) continue;
	  failFlag=0;
          for (i=0; i<set[setNr].anz_nurs; i++)
          {
            if(0>ifind(&set[s].nurs, set[s].anz_nurs, set[setNr].nurs[i] )) break;
          }
          if(i!=set[setNr].anz_nurs ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from setNr are in s included */
            if( compare( mode, "min", 3)==3)  printf(" All nurbs from set:%s also in %s\n", set[setNr].name, set[s].name);
            else /* strict, s must not include more nodes as setNr */
	    {
              if(set[setNr].anz_nurs==set[s].anz_nurs) printf(" set:%s and %s share identical nurbs\n", set[setNr].name, set[s].name);
	    }
	  }
        }
        else /* max */
        {
          if(set[setNr].anz_nurs< set[s].anz_nurs) continue;
	  failFlag=0;
          for (i=0; i<set[s].anz_nurs; i++)
          {
            if(0>ifind(&set[setNr].nurs, set[setNr].anz_nurs, set[s].nurs[i] )) break;
          }
          if(i!=set[s].anz_nurs ) failFlag=1;
          if(!failFlag)
          {
            /* all entities from s are in setNr included */
            printf(" All nurbs from set:%s also in %s\n", set[s].name, set[setNr].name);
	  }
	}
      }

    }
  }
  return(1);
} 


int seti( int setNr, char *type, int sets, char dat[MAX_PARAM_PER_RECORD][MAX_LINE_LENGTH])
{
  int j,s,setnr[100];

  if(sets>100) { printf(" ERROR, to much sets\n"); return(0); }
  for(s=0; s<sets; s++) setnr[s]=getSetNr( dat[s] );
  

  /* check if the entity is a member in all sets */
  if ((type[0]=='s')&&(type[1]=='e'))
  {
    for (j=0; j<set[setnr[0]].anz_n; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].node, set[setnr[s]].anz_n, set[setnr[0]].node[j] )) break;
      }
      if(s==sets) seta( setNr, "n", set[setnr[0]].node[j] );
    }
    for (j=0; j<set[setnr[0]].anz_e; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].elem, set[setnr[s]].anz_e, set[setnr[0]].elem[j] )) break;
      }
      if(s==sets) seta( setNr, "e", set[setnr[0]].elem[j] );
    }
    for (j=0; j<set[setnr[0]].anz_f; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].face, set[setnr[s]].anz_f, set[setnr[0]].face[j] )) break;
      }
      if(s==sets) seta( setNr, "f", set[setnr[0]].face[j] );
    }
    for (j=0; j<set[setnr[0]].anz_p; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].pnt, set[setnr[s]].anz_p, set[setnr[0]].pnt[j] )) break;
      }
      if(s==sets) seta( setNr, "p", set[setnr[0]].pnt[j] );
    }
    for (j=0; j<set[setnr[0]].anz_l; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].line, set[setnr[s]].anz_l, set[setnr[0]].line[j] )) break;
      }
      if(s==sets) seta( setNr, "l", set[setnr[0]].line[j] );
    }
    for (j=0; j<set[setnr[0]].anz_c; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].lcmb, set[setnr[s]].anz_c, set[setnr[0]].lcmb[j] )) break;
      }
      if(s==sets) seta( setNr, "c", set[setnr[0]].lcmb[j] );
    }
    for (j=0; j<set[setnr[0]].anz_s; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].surf, set[setnr[s]].anz_s, set[setnr[0]].surf[j] )) break;
      }
      if(s==sets) seta( setNr, "s", set[setnr[0]].surf[j] );
    }
    for (j=0; j<set[setnr[0]].anz_b; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].body, set[setnr[s]].anz_b, set[setnr[0]].body[j] )) break;
      }
      if(s==sets) seta( setNr, "b", set[setnr[0]].body[j] );
    }
    for (j=0; j<set[setnr[0]].anz_se; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].set, set[setnr[s]].anz_se, set[setnr[0]].set[j] )) break;
      }
      if(s==sets) seta( setNr, "se", set[setnr[0]].set[j] );
    }
    for (j=0; j<set[setnr[0]].anz_sh; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].shp, set[setnr[s]].anz_sh, set[setnr[0]].shp[j] )) break;
      }
      if(s==sets) seta( setNr, "sh", set[setnr[0]].shp[j] );
    }
  }
  else if ((type[0]=='s')&&(type[1]=='h'))
  {
    for (j=0; j<set[setnr[0]].anz_sh; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].shp, set[setnr[s]].anz_sh, set[setnr[0]].shp[j] )) break;
      }
      if(s==sets) seta( setNr, "sh", set[setnr[0]].shp[j] );
    }
  }
  else if (type[0]=='n')
  {
    for (j=0; j<set[setnr[0]].anz_n; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].node, set[setnr[s]].anz_n, set[setnr[0]].node[j] )) break;
      }
      if(s==sets) seta( setNr, "n", set[setnr[0]].node[j] );
    }
  }
  else if (type[0]=='e')
  {
    for (j=0; j<set[setnr[0]].anz_e; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].elem, set[setnr[s]].anz_e, set[setnr[0]].elem[j] )) break;
      }
      if(s==sets) seta( setNr, "e", set[setnr[0]].elem[j] );
    }
  }
  else if (type[0]=='f')
  {
    for (j=0; j<set[setnr[0]].anz_f; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].face, set[setnr[s]].anz_f, set[setnr[0]].face[j] )) break;
      }
      if(s==sets) seta( setNr, "f", set[setnr[0]].face[j] );
    }
  }
  else if (type[0]=='p')
  {
    for (j=0; j<set[setnr[0]].anz_p; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].pnt, set[setnr[s]].anz_p, set[setnr[0]].pnt[j] )) break;
      }
      if(s==sets) seta( setNr, "p", set[setnr[0]].pnt[j] );
    }
  }
  else if (type[0]=='l')
  {
    for (j=0; j<set[setnr[0]].anz_l; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].line, set[setnr[s]].anz_l, set[setnr[0]].line[j] )) break;
      }
      if(s==sets) seta( setNr, "l", set[setnr[0]].line[j] );
    }
  }
  else if (type[0]=='c')
  {
    for (j=0; j<set[setnr[0]].anz_c; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].lcmb, set[setnr[s]].anz_c, set[setnr[0]].lcmb[j] )) break;
      }
      if(s==sets) seta( setNr, "c", set[setnr[0]].lcmb[j] );
    }
  }
  else if (type[0]=='s')
  {
    for (j=0; j<set[setnr[0]].anz_s; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].surf, set[setnr[s]].anz_s, set[setnr[0]].surf[j] )) break;
      }
      if(s==sets) seta( setNr, "s", set[setnr[0]].surf[j] );
    }
  }
  else if (type[0]=='b')
  {
    for (j=0; j<set[setnr[0]].anz_b; j++)
    {
      for(s=1; s<sets; s++)
      {
        if(0>ifind(&set[setnr[s]].body, set[setnr[s]].anz_b, set[setnr[0]].body[j] )) break;
      }
      if(s==sets) seta( setNr, "b", set[setnr[0]].body[j] );
    }
  }
  return(1);
} 


int setr( int setNr, char *type, int number)
{
  int i,j;

  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(-1);
  }
  if( number<0)
  {
    errMsg(" ERROR in setr: A negative entity-index:%d was used\n", number);
    return(-1);
  }

  if (( type[0] == 'r' )||(( type[0] == 's' )&&( type[1] == 'e' )))
  {
    /* set remove  */
    i=iremove(&set[setNr].set, set[setNr].anz_se, number);
    if(i<set[setNr].anz_se)
    {
      set[setNr].anz_se=i;
    }
    else { if(printFlag) printf(" set:%s not in set:%s\n", set[number].name, set[setNr].name); }
  }
  else if (type[0]=='v')
  {
    /* value remove  */
    i=iremove(&set[setNr].valu, set[setNr].anz_v, number);
    if(i<set[setNr].anz_v)
    {
      set[setNr].anz_v=i;
    }
    else { if(printFlag) printf(" value:%s not in set:%s\n", value[number].name, set[setNr].name); }
  }
  else if (( type[0] == 'h' )||(( type[0] == 's' )&&( type[1] == 'h' )))
  {
    /* set remove  */
    i=iremove(&set[setNr].shp, set[setNr].anz_sh, number);
    if(i<set[setNr].anz_sh)
    {
      set[setNr].anz_sh=i;
    }
    else { if(printFlag) printf(" shape:%s not in set:%s\n", shape[number].name, set[setNr].name); }
  }
  else if (type[0]=='n')
  {
    /* node remove  */
    i=iremove(&set[setNr].node, set[setNr].anz_n, number);
    if(i<set[setNr].anz_n)
    {
      set[setNr].anz_n=i;
    }
    else { if(printFlag) printf(" node:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='e')
  {
    /* element remove  */
    i=iremove(&set[setNr].elem, set[setNr].anz_e, number);
    if(i<set[setNr].anz_e)
    {
      set[setNr].anz_e=i;
    }
    else { if(printFlag) printf(" element:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='f')
  {
    /* element remove  */
    i=iremove(&set[setNr].face, set[setNr].anz_f, number);
    if(i<set[setNr].anz_f)
    {
      set[setNr].anz_f=i;
    }
    else { if(printFlag) printf(" face:%d not in set:%s\n", number, set[setNr].name); }
  }
  else if (type[0]=='p')
  {
    /* point remove  */
    i=iremove(&set[setNr].pnt, set[setNr].anz_p, number);
    if(i<set[setNr].anz_p)
    {
      set[setNr].anz_p=i;

      /* remove also embedded nodes */
      for (j=0; j<point[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, point[number].nod[j]);
      }        
    }
    else { if(printFlag) printf(" pnt:%s not in set:%s\n", point[number].name, set[setNr].name); }
  }
  else if (type[0]=='l')
  {
    /* line remove  */
    i=iremove(&set[setNr].line, set[setNr].anz_l, number);
    if(i<set[setNr].anz_l)
    {
      set[setNr].anz_l=i;

      /* remove also embedded nodes */
      for (j=0; j<line[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, line[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<line[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, line[number].elem[j]);
      }        
    }
    else { if(printFlag) printf(" line:%s not in set:%s\n", line[number].name, set[setNr].name); }
  }
  else if (type[0]=='c')
  {
    /* lcmb remove  */
    i=iremove(&set[setNr].lcmb, set[setNr].anz_c, number);
    if(i<set[setNr].anz_c)
    {
      set[setNr].anz_c=i;
    }
    else{  if(printFlag) printf(" lcmb:%s not in set:%s\n", lcmb[number].name, set[setNr].name); }
  }
  else if (type[0]=='s')
  {
    /* surface remove  */
    i=iremove(&set[setNr].surf, set[setNr].anz_s, number);
    if(i<set[setNr].anz_s)
    {
      set[setNr].anz_s=i;

      /* remove also embedded nodes */
      for (j=0; j<surf[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, surf[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<surf[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, surf[number].elem[j]);
      }        
    }
    else{  if(printFlag) printf(" surf:%s not in set:%s\n", surf[number].name, set[setNr].name); }
  }
  else if (type[0]=='b')
  {
    /* body remove  */
    i=iremove(&set[setNr].body, set[setNr].anz_b, number);
    if(i<set[setNr].anz_b)
    {
      set[setNr].anz_b=i;

      /* remove also embedded nodes */
      for (j=0; j<body[number].nn; j++)
      {
        set[setNr].anz_n=iremove(&set[setNr].node, set[setNr].anz_n, body[number].nod[j]);
      }        

      /* remove also embedded elements */
      for (j=0; j<body[number].ne; j++)
      {
        set[setNr].anz_e=iremove(&set[setNr].elem, set[setNr].anz_e, body[number].elem[j]);
      }        
    }
    else{ if(printFlag) printf(" body:%s not in set:%s\n", body[number].name, set[setNr].name); }
  }
  else if (type[0]=='L')
  {
    /* nurl remove  */
    i=iremove(&set[setNr].nurl, set[setNr].anz_nurl, number);
    if(i<set[setNr].anz_nurl)
    {
      set[setNr].anz_nurl=i;
    }
    else{ if(printFlag) printf(" nurl:%s not in set:%s\n", nurbl[number].name, set[setNr].name); }
  }
  else if (type[0]=='S')
  {
    /* nurs remove  */
    i=iremove(&set[setNr].nurs, set[setNr].anz_nurs, number);
    if(i<set[setNr].anz_nurs)
    {
      set[setNr].anz_nurs=i;
    }
    else{ if(printFlag) printf(" nurs:%s not in set:%s\n", nurbs[number].name, set[setNr].name); }
  }
  else if ( type[0] == 'j' )
  {
    if(number<set[setNr].anz_elf-1)
    {
      for ( i=number; i<set[setNr].anz_elf-1; i++)
      {
        set[setNr].elf[number].e=set[setNr].elf[number+1].e;
        set[setNr].elf[number].f=set[setNr].elf[number+1].f;
      }
      if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf)*sizeof(Elfaces))) == NULL )
      { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
      set[setNr].anz_elf--;
    }
    else if(number==set[setNr].anz_elf-1)
    {
      if((set[setNr].elf= (Elfaces *)realloc(set[setNr].elf, (set[setNr].anz_elf)*sizeof(Elfaces))) == NULL )
      { printf("ERROR: realloc failed in seta()\n\n" ); return(-1); }
      set[setNr].anz_elf--;
    }
    else{ if(printFlag) printf(" elf:%d not in set:%s\n", number, set[setNr].name); }
  }
  else
  {
    errMsg ("WARNING: in setr type:%s not recognized\n", type );
    return(-1);
  }
  return(1);
}


/*  seqr works only for sequences (set[].type==1) with points and nodes */
int seqr( int setNr, char *type, int number)
{
  register int i, n;
  static int *puf=NULL;
  int flag=0;

  if( set[setNr].name == (char *)NULL )
  {
    errMsg(" ERROR: setNr:%d is undefined\n", setNr);
    return(0);
  }
  if( number<0)
  {
    errMsg(" ERROR in seqr: A negative entity-index:%d was used\n", number);
    return(-1);
  }

  if ( set[setNr].type!=1)
  {
    errMsg(" FATAL ERROR: in seqr, set:%s is not a sequence\n", set[setNr].name );
  }
  else if (type[0]=='n')
  {
    /* puffer allocieren  */
    if ( (puf = (int *)malloc( (set[setNr].anz_n+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setr\n\n");

    n=0;
    for ( i=0; i<set[setNr].anz_n; i++)
    {
      if (set[setNr].node[i] != number)
      {
        puf[n] = set[setNr].node[i];
        n++;
      }
      else
        flag=1;
    }

    if (flag)
    {
      set[setNr].anz_n--;
      /* if the set is empty, delete the set */
      if((set[setNr].anz_p <1)&& (set[setNr].anz_n <1)) 
      {
        delSet(set[setNr].name); free(puf); return(0);
      }
      if ((set[setNr].node = (int *)realloc( (int *)set[setNr].node,(set[setNr].anz_n+1)*sizeof(int)) ) == NULL)
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); free(puf); return(0); }
      else if(printFlag)
        printf (" set[%d]:%s reallocated and node %d removed\n", setNr, set[setNr].name, number );

      for ( i=0; i<set[setNr].anz_n; i++)
      {
        set[setNr].node[i] = puf[i];
      }
    }
    else if(printFlag) printf(" node:%d not in set:%s\n", number, set[setNr].name);
    free(puf);
  }
  else if (type[0]=='p')
  {
    /* puffer allocieren  */
    if ( (puf = (int *)malloc( (set[setNr].anz_p+1)*sizeof(int))) == NULL )
      printf(" ERROR: malloc failed in setr\n\n");

    n=0;
    for ( i=0; i<set[setNr].anz_p; i++)
    {
      if (set[setNr].pnt[i] != number)
      {
        puf[n] = set[setNr].pnt[i];
        n++;
      }
      else
        flag=1;
    }

    if (flag)
    {
      set[setNr].anz_p--;
      /* if the set is empty, delete the set */
      if((set[setNr].anz_p <1)&& (set[setNr].anz_n <1)) 
      {
        /* suche abhaengige linien, redefine if necessary */
        for (i=0; i<anzGeo->l; i++)
        {
          if(( line[i].typ == 's' )&&( setNr == line[i].trk ))
          {
            line[i].typ = ' ';
          }
        }
        delSet(set[setNr].name); free(puf); return(0);
      }
      if ((set[setNr].pnt = (int *)realloc( (int *)set[setNr].pnt,(set[setNr].anz_p+1)*sizeof(int)) ) == NULL)
      { printf(" ERROR: realloc failed in set[%d]:%s\n\n", setNr, set[setNr].name); free(puf); return(0); }
      else if(printFlag)
        printf (" set[%d]:%s reallocated and pnt %s removed\n", setNr, set[setNr].name, point[number].name );

      for ( i=0; i<set[setNr].anz_p; i++)
      {
        set[setNr].pnt[i] = puf[i];
      }
    }
    else if(printFlag) printf(" pnt:%s not in set:%s\n", point[number].name, set[setNr].name);
    free(puf);
  }
  return(1);
}


double calcLineLength(int l)
{
  int n;
  double p1[3], p2[3], p1p2[3], lp1p2, lmax=0.;

  repLine(l);
  p1[0]=line[l].ip[0]*scale->w+scale->x;
  p1[1]=line[l].ip[1]*scale->w+scale->y;
  p1[2]=line[l].ip[2]*scale->w+scale->z;
  for (n=3; n<line[l].nip; n+=3)
  {
    p2[0]=line[l].ip[n]*scale->w+scale->x;
    p2[1]=line[l].ip[n+1]*scale->w+scale->y;
    p2[2]=line[l].ip[n+2]*scale->w+scale->z;
    v_result( p1, p2, p1p2 );
    lp1p2=v_betrag( p1p2 );
    lmax+=lp1p2;

    p1[0]=p2[0];
    p1[1]=p2[1];
    p1[2]=p2[2];
  }
  return(lmax);
}


/* returns 0 if commands are to be skipped */
int pre_while(char *type, char *string, int *na, int *nb, FILE *handle1, int *addFlag, int *gtolFlag, int wFlag)
{
  typedef struct{
    int flag;
    int clines;
    int lineptr;
    int prev_ptr;
    int next_ptr;
    char arg1[MAX_LINE_LENGTH];
    char arg2[MAX_LINE_LENGTH];
    char operator[MAX_LINE_LENGTH];
    char **type;
    char **string;
    int *na;
    int *nb;
    FILE **handle1;
    int *addFlag;
    int *gtolFlag;
  }whileStack;

  static whileStack *stack=NULL;
  static int stack_ptr=-1;             // active while loop
  static int next_ptr=-1;              // next while loop
  static int nwhiles=0;             // allocated loops
  int   i,j;
  int   flag;
  char  arg1[MAX_LINE_LENGTH], operator[MAX_LINE_LENGTH], arg2[MAX_LINE_LENGTH];

  //printf("hallo in pre_while: type:%s str:%s nab:%d %d addf:%d gtolf:%d ",type,string,*na,*nb,*addFlag,*gtolFlag);
  //printf("hallo whileFlag:%d\n",wFlag);

  sscanf(string, "%*s %s %s %s", arg1, operator, arg2);

  // store the command
  // wFlag==2,4: save the command (makes here no difference)
  if((wFlag==2)||(wFlag==4))
  {
    if ((stack[stack_ptr].type = (char **)realloc(stack[stack_ptr].type, (stack[stack_ptr].clines+1)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    if ((stack[stack_ptr].type[stack[stack_ptr].clines] = (char *)malloc( (strlen(type)+1)*sizeof(char)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    strcpy(stack[stack_ptr].type[stack[stack_ptr].clines], type);

    if ((stack[stack_ptr].string = (char **)realloc(stack[stack_ptr].string, (stack[stack_ptr].clines+1)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    if ((stack[stack_ptr].string[stack[stack_ptr].clines] = (char *)malloc( (strlen(string)+1)*sizeof(char)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    strcpy(stack[stack_ptr].string[stack[stack_ptr].clines], string);

    if ((stack[stack_ptr].na = (int *)realloc(stack[stack_ptr].na, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].na[stack[stack_ptr].clines]=*na;

    if ((stack[stack_ptr].nb = (int *)realloc(stack[stack_ptr].nb, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].nb[stack[stack_ptr].clines]=*nb;

    if ((stack[stack_ptr].handle1 = (FILE **)realloc(stack[stack_ptr].handle1, (stack[stack_ptr].clines+1)*sizeof(FILE *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].handle1[stack[stack_ptr].clines]=handle1;

    if ((stack[stack_ptr].addFlag = (int *)realloc(stack[stack_ptr].addFlag, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].addFlag[stack[stack_ptr].clines]=*addFlag;

    if ((stack[stack_ptr].gtolFlag = (int *)realloc(stack[stack_ptr].gtolFlag, (stack[stack_ptr].clines+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, whilestack\n\n"); return(1); }
    stack[stack_ptr].gtolFlag[stack[stack_ptr].clines]=*gtolFlag;

    stack[stack_ptr].clines++;

    //printf("hallo stored string:%s stack_ptr:%d clines:%d \n", string, stack_ptr, stack[stack_ptr].clines);

    return(wFlag);
  }

  // restore the next command
  else if(wFlag==3)
  {
    if(stack[stack_ptr].lineptr<stack[stack_ptr].clines)
    {
      strcpy(type,stack[stack_ptr].type[stack[stack_ptr].lineptr]);
      strcpy(string,stack[stack_ptr].string[stack[stack_ptr].lineptr]);
      *na=stack[stack_ptr].na[stack[stack_ptr].lineptr];
      *nb=stack[stack_ptr].nb[stack[stack_ptr].lineptr];
      handle1=stack[stack_ptr].handle1[stack[stack_ptr].lineptr];
      *addFlag=stack[stack_ptr].addFlag[stack[stack_ptr].lineptr];
      *gtolFlag=stack[stack_ptr].gtolFlag[stack[stack_ptr].lineptr];
      stack[stack_ptr].lineptr++;
    }
    else { printf("ERROR: in while, no more commands (%d from %d stack_ptr:%d\n", stack[stack_ptr].lineptr, stack[stack_ptr].clines, stack_ptr); exit(0); }

    //printf("hallo restored line:%d  %s\n", stack[stack_ptr].lineptr, string );
    return(wFlag);
  }

  else if(compare( type,"WHILE",2)==2 )
  {
    i=getValuNr(arg1);
    if (i>-1) strcpy(arg1,value[i].string );
    i=getValuNr(arg2);
    if (i>-1) strcpy(arg2,value[i].string );

    if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=3; else flag=1; }
    else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=3; else flag=1; }
    else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=3; else flag=0; }
    else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=3; else flag=0; }
    else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=3; else flag=0; }
    else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=3; else flag=0; }
    else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 

    //printf("hallo WHILE %s nwhiles:%d stackptr:%d flag:%d\n", string, nwhiles, stack_ptr, flag);
    if(flag==1) return(1);  // error


    if((stack_ptr<0)||(stack[0].flag==2)||(stack[0].flag==4)) // create new while-stacks as long as the main loop has not looped 
    {
      if ((stack = (whileStack *)realloc( (whileStack *)stack, (nwhiles+1)*sizeof(whileStack)) ) == NULL )
      { printf("\n\nERROR: realloc failure, whileStack\n\n"); return(1); }
      if(flag==0) stack[nwhiles].flag=4;  // store but not execute
      if(flag==3) stack[nwhiles].flag=2;  // store and execute
      stack[nwhiles].clines=0;
      stack[nwhiles].prev_ptr=stack_ptr;
      stack[nwhiles].next_ptr=nwhiles+1;
      sscanf(string, "%*s %s %s %s", arg1, operator, arg2);
      strcpy(stack[nwhiles].arg1,arg1);
      strcpy(stack[nwhiles].arg2,arg2);
      strcpy(stack[nwhiles].operator,operator);
      stack[nwhiles].type=NULL;
      stack[nwhiles].string=NULL;
      stack[nwhiles].na=NULL;
      stack[nwhiles].nb=NULL;
      stack[nwhiles].handle1=NULL;
      stack[nwhiles].addFlag=NULL;
      stack[nwhiles].gtolFlag=NULL;
      stack_ptr=nwhiles;
      return(stack[nwhiles++].flag);
    }

    //if stack[0].flag==3 (all commands are stored and repeated)
    //  for seriell whiles the stack_ptr has to be increased by the amount of seriell whiles
    stack_ptr=next_ptr;        // set the pointer to the next while
    if(flag==0) stack[stack_ptr].flag=0;  // do not restore and do not not execute
    if(flag==3) stack[stack_ptr].flag=3;    // restore and execute
    return(stack[stack_ptr].flag);
  }

  else if(compare( type,"ENDWHILE",2)==2 )
  {
    // if the while was skipped from the beginning continue executing
    if((stack[stack_ptr].flag == 0)||(stack[stack_ptr].flag == 4))
    {
      //printf(" ENDWHILE stackptr:%d\n",stack_ptr);
      // for seriell whiles the stack_ptr has to be decreased by the amount of seriell whiles
      stack[stack_ptr].lineptr=0;
      stack_ptr = stack[stack_ptr].prev_ptr;
      if(stack_ptr>=0)
        return(stack[stack_ptr].flag);
      else return(1);
    }
    else
    {
      strcpy(arg1,stack[stack_ptr].arg1);
      strcpy(arg2,stack[stack_ptr].arg2);
      strcpy(operator,stack[stack_ptr].operator);

      i=getValuNr(arg1);
      if (i>-1) strcpy(arg1,value[i].string );
      i=getValuNr(arg2);
      if (i>-1) strcpy(arg2,value[i].string );
  
      if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=3; else flag=1; }
      else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=3; else flag=1; }
      else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=3; else flag=1; }
      else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=3; else flag=1; }
      else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=3; else flag=1; }
      else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=3; else flag=1; }
      else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 

      //printf(" ENDWHILE eval: %s %s %s == flag:%d\n",arg1,operator,arg2, flag);

      stack[stack_ptr].flag=flag;
      stack[stack_ptr].lineptr=0;
      next_ptr=stack[stack_ptr].next_ptr;

      // if finished
      if(flag==1)
      {
        stack_ptr = stack[stack_ptr].prev_ptr;

        // free if stack_ptr is <0 (first while is closed)
	if(stack_ptr<0)
	{
	  for(i=0; i<nwhiles; i++)
	  {
	    for(j=0; j<stack[i].clines; j++)
	    {
              free(stack[i].type[j]);
              free(stack[i].string[j]);
            }
            free(stack[i].type);
            free(stack[i].string);
            free(stack[i].na);
            free(stack[i].nb);
            free(stack[i].handle1);
            free(stack[i].addFlag);
            free(stack[i].gtolFlag);
            stack[i].type=NULL;
            stack[i].string=NULL;
            stack[i].na=NULL;
            stack[i].nb=NULL;
            stack[i].handle1=NULL;
            stack[i].addFlag=NULL;
            stack[i].gtolFlag=NULL;
	  }
          nwhiles=0;
	}
        if(stack_ptr>=0)
          return(stack[stack_ptr].flag);
        else return(1);
      }
      // condition fulfilled, repeat the commands
      return(3);
    }
  }

  return(0);
}


/* returns 0 if commands are to be skipped */
int pre_if(char *string, int ifFlag)
{
  static int *stack=NULL;
  static int stack_ptr=0;
  int i, flag;
  char type[MAX_LINE_LENGTH], arg1[MAX_LINE_LENGTH], operator[MAX_LINE_LENGTH], arg2[MAX_LINE_LENGTH];

  //printf("if: %s\n",string);
  sscanf(string, "%s %s %s %s", type, arg1, operator, arg2);
  for(i=0;i<strlen(type); i++) type[i]=toupper(type[i]);

  if(compare( type,"IF",2)==2 )
  {
    if((operator[0]=='e')&&(operator[1]=='q')) { if(compareStrings(arg1,arg2)>0) flag=1; else flag=0; }
    else if((operator[0]=='n')&&(operator[1]=='e')) { if(compareStrings(arg1,arg2)<1) flag=1; else flag=0; }
    else if((operator[0]=='=')&&(operator[1]=='=')) { if(atof(arg1)==atof(arg2)) flag=1; else flag=0; }
    else if((operator[0]=='!')&&(operator[1]=='=')) { if(atof(arg1)!=atof(arg2)) flag=1; else flag=0; }
    else if(operator[0]=='>') { if(atof(arg1)>atof(arg2)) flag=1; else flag=0; }
    else if(operator[0]=='<') { if(atof(arg1)<atof(arg2)) flag=1; else flag=0; }
    else { printf("ERROR: operator not known:%s\n", operator); flag=1; } 

    //printf(" IF flag:%d\n",flag);

    if ((stack = (int *)realloc( (int *)stack, (stack_ptr+1)*sizeof(int)) ) == NULL )
    { printf("\n\nERROR: realloc failure, ifstack\n\n"); return(1); }
    if(ifFlag) stack[stack_ptr]=flag;
    else stack[stack_ptr]=0;
    return(stack[stack_ptr++]);
  }
  else if(compare( type,"ELSE",2)==2 )
  {
    //printf(" ELSE:stack_ptr:%d\n",(stack[stack_ptr-1]));
    if((stack_ptr>1)&&(stack[stack_ptr-2] == 0)) return(0);
    if(stack_ptr>0) return(!stack[stack_ptr-1]);
  }
  else if(compare( type,"ENDIF",2)==2 )
  {
    //printf(" ENDIF stackptr:%d\n",stack_ptr-1);
    if((stack_ptr>1)&&(stack[stack_ptr-2] == 0)) flag=0; else flag=1;
    if(stack_ptr>0) stack_ptr--;
    return(flag);
  }

  return(0);
}


double pre_length(char *setname)
{
  int   i,n;
  int   setNr,l;
  double L=0, Ll=0;
  //double sum_valLe=0., value=0.;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }

  for(i=0; i<set[setNr].anz_l; i++)
  {
    l=set[setNr].line[i];
    Ll=calcLineLength(l);
    L+=Ll;
  }
  printf("LENGTH:%lf", L);
  //if(anz->l) printf(" AVERAGE-VALUE:%f\n", sum_valLe/vol);
  //else
    printf("\n");
  if(valuestackFlag)
  {
    n=1;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    sprintf(valuestack[valuestack_ptr++],"%f", L );
    printf(" written to stack\n");
  }

  return(L);
}


double pre_area(char *setname)
{
  int   i,j,n;
  int   nr, setNr;
  double A=0., Ae, Abuf, p[3][3], pcg[3], Ix, Iy, Ixy;
  double xcg=0., ycg=0., zcg=0.;
  double sum_valAe=0., value=0.;

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }
  if (set[setNr].anz_f<1)
  {
    printf (" ERROR: set:%s does not contain faces\n", setname);
    return(-1);
  }

  for(i=0; i<set[setNr].anz_f; i++)
  {
    Abuf=A;
    nr=set[setNr].face[i];
    switch(face[nr].type)
    {
      case 7:
      for(j=0; j<3; j++)
      {
        p[j][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[j][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[j][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 8:
      p[0][0]=node[face[nr].nod[0]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[0]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[0]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[1]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[1]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[1]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[2]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[2]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[2]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 9:
      for(j=0; j<3; j++)
      {
        p[j][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[j][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[j][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      n=j=0;
      p[n][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
      p[n][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
      p[n++][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      for(j=2; j<4; j++)
      {
        p[n][0]=node[face[nr].nod[j]].nx*scale->w+scale->x;
        p[n][1]=node[face[nr].nod[j]].ny*scale->w+scale->y;
        p[n++][2]=node[face[nr].nod[j]].nz*scale->w+scale->z;
      }
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      case 10:
      p[0][0]=node[face[nr].nod[0]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[0]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[0]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }

      p[0][0]=node[face[nr].nod[7]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[7]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[7]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[3]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[3]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[3]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[1]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[1]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[1]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      p[0][0]=node[face[nr].nod[8]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[8]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[8]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[4]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[4]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[4]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }

      p[0][0]=node[face[nr].nod[5]].nx*scale->w+scale->x;
      p[0][1]=node[face[nr].nod[5]].ny*scale->w+scale->y;
      p[0][2]=node[face[nr].nod[5]].nz*scale->w+scale->z;
      p[1][0]=node[face[nr].nod[2]].nx*scale->w+scale->x;
      p[1][1]=node[face[nr].nod[2]].ny*scale->w+scale->y;
      p[1][2]=node[face[nr].nod[2]].nz*scale->w+scale->z;
      p[2][0]=node[face[nr].nod[6]].nx*scale->w+scale->x;
      p[2][1]=node[face[nr].nod[6]].ny*scale->w+scale->y;
      p[2][2]=node[face[nr].nod[6]].nz*scale->w+scale->z;
      if(getGeoDataTria( p[0], p[1], p[2], &Ix, &Iy, &Ixy, &Ae, pcg)!=0)
      {
        A+=Ae;
        xcg+=pcg[0]*Ae;
        ycg+=pcg[1]*Ae;
        zcg+=pcg[2]*Ae;
      }
      break;


      printf("ERROR: type:%d of face:%d not known. Interrupt\n",face[nr].type,nr); 
      return(-1.);
    }

    /* determine the average node-value */
    if(anz->l)
    {
      if (face[nr].type == 7) n = 3;  /* TRI3  */
      else if (face[nr].type == 8) n = 6;  /* TRI6  */
      else if (face[nr].type == 9) n = 4;  /* QUAD4 */
      else if (face[nr].type == 10) n = 8; /* QUAD8 */
      else if (face[nr].type == 11) n = 2; /* beam2 */
      else if (face[nr].type == 12) n = 3; /* beam3 */
      else n=0;
      value=0.;
      for (j=0; j<n; j++)
      {
        if(sequenceFlag) value+=lcase[lcase_animList].dat[animList][face[nr].nod[j]];
        else value+=lcase[cur_lc].dat[cur_entity][face[nr].nod[j]];
      }
      sum_valAe+=value/n*(A-Abuf);
    }
  }
  if(anz->l) printf("AREA:%lf  CENTER OF GRAVITY: %lf %lf %lf AVERAGE-VALUE:%f\n", A,xcg/A,ycg/A,zcg/A, sum_valAe/A);
  else   printf("AREA:%lf  CENTER OF GRAVITY: %lf %lf %lf\n", A,xcg/A,ycg/A,zcg/A);

  if(valuestackFlag)
  {
    if(anz->l) n=5; else n=4;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    if(anz->l) sprintf(valuestack[valuestack_ptr++],"%e", sum_valAe/A );
    sprintf(valuestack[valuestack_ptr++],"%e", zcg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", ycg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", xcg/A );
    sprintf(valuestack[valuestack_ptr++],"%e", A );
    printf(" in inverse order written to stack\n");
  }

  return(A);
}



double pre_volu(char *setname)
{
  int   i,j,n;
  int   nr, setNr, massFlag=1, elFlag=0;
  int   istat[3]={0,0,0};
  double vol=0., mass=0., vole, masse, x[20],y[20],z[20];
  double xcg=0., ycg=0., zcg=0., xcge, ycge, zcge;
  double sum_valVe=0., value=0.;
  double xl[20][3], cg[3];
  char elty[MAX_LINE_LENGTH];

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return(-1);
  }

  for(i=0; i<set[setNr].anz_e; i++)
  {
    nr=set[setNr].elem[i];
    if(e_enqire[nr].type==1)
    {
      for(j=0; j<8; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D8");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //printf("vol:%e cg: %f %f %f\n", vole,xcge,ycge,zcge);
      //return(0);
    }
    else if(e_enqire[nr].type==4)
    {
      for(j=0; j<12; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=16; n<20; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=12; n<16; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D20");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);
    }
    else if(e_enqire[nr].type==2)
    {
      for(j=0; j<6; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D6");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);

      /* calc as collapsed hex */
      /*
      for(j=0; j<3; j++)
      {
        x[j]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      x[j]=node[e_enqire[nr].nod[2]].nx*scale->w+scale->x;
      y[j]=node[e_enqire[nr].nod[2]].ny*scale->w+scale->y;
      z[j]=node[e_enqire[nr].nod[2]].nz*scale->w+scale->z;
      for(j=3; j<6; j++)
      {
        x[j+1]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j+1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j+1]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      x[7]=node[e_enqire[nr].nod[5]].nx*scale->w+scale->x;
      y[7]=node[e_enqire[nr].nod[5]].ny*scale->w+scale->y;
      z[7]=node[e_enqire[nr].nod[5]].nz*scale->w+scale->z;
      hexaeder_(&nr, istat, x,y,z, &vole, &xcge, &ycge, &zcge );
*/
    }
    else if(e_enqire[nr].type==5)
    {
      for(j=0; j<9; j++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[j]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[j]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[j]].nz* scale->w+scale->z;
      }
      for(n=12; n<15; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      for(n=9; n<12; n++)
      { 
        xl[j][0]= node[e_enqire[set[setNr].elem[i]].nod[n]].nx* scale->w+scale->x;
        xl[j][1]= node[e_enqire[set[setNr].elem[i]].nod[n]].ny* scale->w+scale->y;
        xl[j][2]= node[e_enqire[set[setNr].elem[i]].nod[n]].nz* scale->w+scale->z;
        j++;
      }
      strcpy(elty,"C3D15");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);

      /* calc as collapsed hex */
      /*
      for(j=0; j<3; j++)
      {
        x[j]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      x[j]=node[e_enqire[nr].nod[2]].nx*scale->w+scale->x;
      y[j]=node[e_enqire[nr].nod[2]].ny*scale->w+scale->y;
      z[j]=node[e_enqire[nr].nod[2]].nz*scale->w+scale->z;
      for(j=3; j<6; j++)
      {
        x[j+1]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j+1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j+1]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      x[7]=node[e_enqire[nr].nod[5]].nx*scale->w+scale->x;
      y[7]=node[e_enqire[nr].nod[5]].ny*scale->w+scale->y;
      z[7]=node[e_enqire[nr].nod[5]].nz*scale->w+scale->z;
      hexaeder_(&nr, istat, x,y,z, &vole, &xcge, &ycge, &zcge );
      */
    }
    else if(e_enqire[nr].type==3)
    {
      for(j=0; j<4; j++)
      {
        x[j]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        y[j]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        z[j]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
      }
      tetraeder_(&nr, istat, x,y,z, &vole, &xcge, &ycge, &zcge );
    }
    else if(e_enqire[nr].type==6)
    {
      for(j=0; j<10; j++)
      {
        xl[j][0]=node[e_enqire[nr].nod[j]].nx*scale->w+scale->x;
        xl[j][1]=node[e_enqire[nr].nod[j]].ny*scale->w+scale->y;
        xl[j][2]=node[e_enqire[nr].nod[j]].nz*scale->w+scale->z;
	//printf("xyz: %f %f %f\n", xl[j][0],xl[j][1],xl[j][2]);
      }
      strcpy(elty,"C3D10");
      e_c3d_volu_(&xl[0][0], elty, &vole, cg);
      xcge=cg[0];
      ycge=cg[1];
      zcge=cg[2];
      //return(0);
    }
    else
    {
      printf("ERROR: type:%d of elem:%d not known. Interrupt\n",e_enqire[nr].type,nr); 
      return(-1.);
    }

    /* determine the average node-value */
    if(anz->l)
    {
      if (e_enqire[nr].type == 1) n = 8;  /* HEXA8 */
      else if (e_enqire[nr].type == 2) n = 6;  /* PENTA6 */
      else if (e_enqire[nr].type == 3) n = 4;  /* TET4 */
      else if (e_enqire[nr].type == 4) n = 20; /* HEX20 */
      else if (e_enqire[nr].type == 5) n = 15; /* PENTA15 */
      else if (e_enqire[nr].type == 6) n = 10; /* TET10 */
      else if (e_enqire[nr].type == 7) n = 3;  /* TRI3  */
      else if (e_enqire[nr].type == 8) n = 6;  /* TRI6  */
      else if (e_enqire[nr].type == 9) n = 4;  /* QUAD4 */
      else if (e_enqire[nr].type == 10) n = 8; /* QUAD8 */
      else if (e_enqire[nr].type == 11) n = 2; /* BEAM */
      else if (e_enqire[nr].type == 12) n = 3; /* BEAM3 */
      else n=0;
      value=0.;
      for (j=0; j<n; j++)
      {
        if(sequenceFlag) value+=lcase[lcase_animList].dat[animList][e_enqire[nr].nod[j]];
        else value+=lcase[cur_lc].dat[cur_entity][e_enqire[nr].nod[j]];
      }
      sum_valVe+=value/n*(vole);
    }

    /*
       Berechnung globaler Größen:
    */

    /* get material properties */
    masse=-MAX_FLOAT;
    elFlag=-1;
    for(j=0; j<anz->sets; j++)
    {
      if((set[j].name!=(char *)NULL) && (!set[j].type) && (set[j].name[0]!='-') && (set[j].material>-1)) 
      {
        if(( getIndex(&set[j].elem,set[j].anz_e,nr) >-1) && (material[set[j].material].rho>-1.))
        {
          if(elFlag!=-1)
          {
            printf("ERROR: found material-definition for elem:%d in set:%s and %s\n", nr, set[j].name,set[elFlag].name );  
            break;
          }
          elFlag=j;
          masse= vole * material[set[j].material].rho;
          mass+= masse;
        }
      }
    }
    if (masse==-MAX_FLOAT)
    {
      massFlag=0;
      masse=vole;
    }
              
    vol+=vole;
    xcg+=xcge*masse;
    ycg+=ycge*masse;
    zcg+=zcge*masse;
  }
  if((mass!=0.)&&(!massFlag))
  {
    printf("ERROR: some elements have no material assigned, either none or all need a material reference\n");
  }
  else
  {
    if (massFlag)     printf("VOLUME:%lf  MASS:%e CENTER OF GRAVITY: %lf %lf %lf", vol,mass,xcg/mass,ycg/mass,zcg/mass);
    else printf("VOLUME:%lf CENTER OF GRAVITY: %lf %lf %lf", vol,xcg/vol,ycg/vol,zcg/vol);
    if(anz->l) printf(" AVERAGE-VALUE:%f\n", sum_valVe/vol); else printf("\n");
  }

  if(valuestackFlag)
  {
    if(anz->l) n=5; else n=4;
    if (massFlag) n++;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    if(anz->l) sprintf(valuestack[valuestack_ptr++],"%e", sum_valVe/vol );
    if (massFlag)
    {
      sprintf(valuestack[valuestack_ptr++],"%e", zcg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", ycg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", xcg/mass );
      sprintf(valuestack[valuestack_ptr++],"%e", mass );
    }
    else
    {
      sprintf(valuestack[valuestack_ptr++],"%e", zcg/vol );
      sprintf(valuestack[valuestack_ptr++],"%e", ycg/vol );
      sprintf(valuestack[valuestack_ptr++],"%e", xcg/vol );
    }
    sprintf(valuestack[valuestack_ptr++],"%e", vol );
    printf(" in inverse order written to stack\n");
  }
  return(vol);
}


void completeFacesByTolerance(int set_highl, int setNr, double qaddTol)
{
  register int i,j,jj,jjj,jjjj;
  int f,n,n2,k,l, equ;

  int sumMaster=0;

  int *facebuf;  // stores the identified faces which fulfill the criterion
  int *fchecked; // 1: if a certain face was already checked
  double **fnorm;  // average normal on the face
  double criterion; 
  double v1[3], v2[3], sprod;
  Nodes *norm;
  int   *sum_n, nmax;

  typedef struct {
    int sum;
    int *face;   // face which uses this node (lowest node-nr in face
  }Fnode;
  Fnode *fnode;  // field indexed by the lowest node-nr of a face


  criterion=qaddTol*PI/180.;

  /*
  Go over all master-faces and check the neighbours against the criterion of qaddTol.
  If a face passes it will also be a master face.
  The ammount of masters will therefore increase up to a certain level.
  The comparison stops if no master is left.
  */

  if ( ( fnode= (Fnode *)malloc((anz->nmax+1) * sizeof(Fnode))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<=anz->nmax; i++) { fnode[i].face=NULL; fnode[i].sum=0; }
  if ( ( facebuf= (int *)malloc((set[setNr].anz_f+1) * sizeof(int))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  if ( ( fchecked= (int *)malloc((anz->f+1) * sizeof(int))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  if ( ( fnorm= (double **)malloc((anz->f+1) * sizeof(double *))) == NULL )
    printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<anz->f; i++)
    if ( ( fnorm[i]= (double *)calloc((3), sizeof(double))) == NULL )
      printf("\n\n ERROR: realloc failed\n");
  for(i=0; i<anz->f; i++)
  {
    fchecked[i]=0;
  }
  for(i=0; i<set[setNr].anz_f; i++)
  {
    facebuf[i]=set[setNr].face[i];
    fchecked[set[setNr].face[i]]=1;
  }
  sumMaster=set[setNr].anz_f;

  /* calc the average normal on all faces */

  /* calculate the average normal on every node */
  getNodeNormalen(&sum_n, &norm, setall, anz, face);
  for (f=0; f<anz->f; f++)
  {
    if (face[f].type == 7) n = 3;  /* TRI3  */
    else if (face[f].type == 8) n = 3;  /* TRI6  */
    else if (face[f].type == 9) n = 4;  /* QUAD4 */
    else if (face[f].type == 10) n = 4; /* QUAD8 */
    else if (face[f].type == 11) n = 2; /* BEAM2 */
    else if (face[f].type == 12) n = 2; /* BEAM3 */
    else n=0;
    if(n==2)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[1]].nx, fnorm[f]);
    }
    if(n==3)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[1]].nx, v1);
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[2]].nx, v2);
      v_prod( v1, v2, fnorm[f] );
    }
    if(n==4)
    {
      v_result( &node[face[f].nod[0]].nx, &node[face[f].nod[2]].nx, v1);
      v_result( &node[face[f].nod[1]].nx, &node[face[f].nod[3]].nx, v2);
      v_prod( v1, v2, fnorm[f] );
    }
    v_norm(fnorm[f],fnorm[f]);

    /* get the faces at each node */
    for(j=0;j<n; j++)
    {
      nmax=face[f].nod[j];
      if ( ( fnode[nmax].face= (int *)realloc((int *)fnode[nmax].face, (fnode[nmax].sum+1)*sizeof(int))) == NULL )
        printf("\n\n ERROR: realloc failed\n");
      fnode[nmax].face[fnode[nmax].sum]=f;
      fnode[nmax].sum++;
    }
  }

  /* go over all master-faces and search and check the slaves */
  for(i=0; i<sumMaster; i++)
  {
    f=facebuf[i];

    /* nodes of the master */
    if (face[f].type == 7) n = 3;  /* TRI3  */
    else if (face[f].type == 8) n = 3;  /* TRI6  */
    else if (face[f].type == 9) n = 4;  /* QUAD4 */
    else if (face[f].type == 10) n = 4; /* QUAD8 */
    else if (face[f].type == 11) n = 2; /* BEAM2 */
    else if (face[f].type == 12) n = 2; /* BEAM3 */
    else n=0;

    for(jjj=0;jjj<n; jjj++)
    {
     nmax=face[f].nod[jjj];
     /* identify the neighbour faces */
     for (jj=0; jj<fnode[nmax].sum; jj++)
     {
      j=fnode[nmax].face[jj];
      if (face[j].type == 7) n2 = 3;  /* TRI3  */
      else if (face[j].type == 8) n2 = 3;  /* TRI6  */
      else if (face[j].type == 9) n2 = 4;  /* QUAD4 */
      else if (face[j].type == 10) n2 = 4; /* QUAD8 */
      else if (face[j].type == 11) n2 = 2; /* BEAM2 */
      else if (face[j].type == 12) n2 = 2; /* BEAM3 */
      else n2=0;
      equ=0;
      for (k=0; k<n2; k++) { for (l=0; l<n; l++)  if(face[f].nod[l] == face[j].nod[k]) equ++; }
      //if(j==91472) printf("i:%d master:%d m:%d s:%d equ:%d check:%d\n", i, sumMaster, f, j, equ, fchecked[j]);
      if((equ==2) || ((n2==2)&&(equ==1)))
      {
        /* check j against the criterion */
        sprod=abs(v_sprod(fnorm[f], fnorm[j]));
        if(sprod>1.) sprod=1.;
	/*
	if(j==91472){
	  printf("i:%d sum:%d S:%f angle %f between %d %d, criterion:%f  check:%d\n", i, sumMaster, sprod, acos(sprod), f, j, criterion, fchecked[j]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][0], fnorm[j][0]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][1], fnorm[j][1]);
	printf("norm master:%f neighbor: %f \n", fnorm[f][2], fnorm[j][2]);
	}
	*/
        if((!fchecked[j])&&(acos(sprod) < criterion))
	{
          /* add j as additional masterface */
          if ( ( facebuf= (int *)realloc((int *)facebuf, (sumMaster+1) * sizeof(int))) == NULL )
            printf("\n\n ERROR: realloc failed\n");
          facebuf[sumMaster]=j;
          sumMaster++; 
          fchecked[j]=1;
    printf ("%d e:%d s:%d n= ", j, face[j].elem_nr, face[j].nr+1 );
      if(face[j].type==7) k=3;
      else if(face[j].type==8) k=6;
      else if(face[j].type==9) k=4;
      else if(face[j].type==10) k=8;
      else k=0;
      for (jjjj=0; jjjj<k; jjjj++) printf("%d ",face[j].nod[jjjj]);
      printf("\n"); 
          seta( setNr, "f", j );
          seta(set_highl, "f", j );
	}
      }
     }
    }
  }
}

void separateMeshes( char *setName)
{
  int i,e,k=2;
  int setNr, setNrbuf, setTmp, sum_c=0, c1, sum1,sum2;
  int *elUsed;
  char setname[MAX_LINE_LENGTH];
  char buffer[MAX_LINE_LENGTH];

  printf (" please wait for 'ready'\n");

  setNr=getSetNr(setName);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setName);
    return;
  }

  if (set[setNr].anz_e<2)
  {
    //printf("ERROR: set:%s contains less than 2 elements!\n", set[setNr].name);
    return;
  }
  /*----- complete set with all elements and nodes ---*/

  /* mark all elements as unused */
  if((elUsed = (int *)malloc( (int)(anz->emax+1)*sizeof(int)))==NULL)
  { errMsg("\n\n ERROR: malloc failed for elUsed\n" ); return; }
  for(i=0; i<=anz->emax; i++) elUsed[i]=0;

  /* use "comp set up/do" until no more entities are added */
  e=set[setNr].elem[0];
  do
  {
    sprintf( setname, "%s%d", specialset->cf,sum_c+1);
    delSet(setname);
    c1=pre_seta(setname,"i", 0);

    sprintf(buffer,"%d", e);
    delSet("+buf");
    setNrbuf=pre_seta("+buf","e", buffer);
    completeSet_Mesh(  setNrbuf, 0, elUsed, 1);                      // down, nodes from new elems in +buf
    for(i=0; i<set[setNrbuf].anz_n; i++)
    {
      seta(c1,"n", set[setNrbuf].node[i]);
    }

    seta(c1,"e", e);
    elUsed[e]=1; 
    do
    {
      sum1=set[c1].anz_n+set[c1].anz_e;

      delSet("+buf2");
      setTmp=pre_seta("+buf2", "i", 0 );
      completeSet_Mesh( setNrbuf, setTmp, elUsed, 0);                  // up, only new elems in +buf2 (setTmp) based on nodes in +buf (setNrbuf)

      completeSet_Mesh(  setTmp, 0, elUsed, 1);                      // down, nodes from new elems in +buf2 (setTmp)

      for(i=0; i<set[setTmp].anz_e; i++)
      {
        seta(c1,"e", set[setTmp].elem[i]);
        elUsed[set[setTmp].elem[i]]=1; 
      }

      delSet("+buf");
      setNrbuf=pre_seta("+buf","i", 0);
      for(i=0; i<set[setTmp].anz_n; i++)
      {
        seta(c1,"n", set[setTmp].node[i]);
        seta(setNrbuf,"n", set[setTmp].node[i]);
      }
      sum2=set[c1].anz_n+set[c1].anz_e;
    }while(sum1<sum2);

    //completeSet( setname, "f");                  // faces (draw-nodes are missed)
    completeSet( setname, "do");                  // faces, nodes (to include also the additional midface-nodes for drawing)

    if(k>=SET_COLS) k=BAS_COLS-1;
    printf("set%d: %s col:%c\n",sum_c+1, setname, entity_k[k]);
    sprintf(buffer,"f %s %c", setname, entity_k[k++]);
    if(!sum_c) plot(buffer);
    else plus(buffer);

    sum_c++;
    e=0;
    for(i=0; i<set[setNr].anz_e; i++)
    {
      if(elUsed[set[setNr].elem[i]]==0)
      {
        e=set[setNr].elem[i];
        break;
      }
    }
  }while(e);

  free(elUsed);
  delSet("+buf");
  delSet("+buf2");

  printf (" ready\n");
}




void separateLines( char *setName)
{
  int i,e,k=2;
  int setNr, setNrbuf, setTmp, sum_c=0, c1, sum1,sum2;
  int *elUsed;
  char setname[MAX_LINE_LENGTH];
  char buffer[MAX_LINE_LENGTH];

  printf (" please wait for 'ready'\n");

  setNr=getSetNr(setName);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setName);
    return;
  }

  if (set[setNr].anz_l<2)
  {
    //printf("ERROR: set:%s contains less than 2 lines!\n", set[setNr].name);
    return;
  }
  /*----- complete set with all lines and points ---*/

  /* mark all lines as unused */
  if((elUsed = (int *)malloc( (int)(anzGeo->l+1)*sizeof(int)))==NULL)
  { errMsg("\n\n ERROR: malloc failed for elUsed\n" ); return; }
  for(i=0; i<=anzGeo->l; i++) elUsed[i]=0;

  /* use "comp set up/do" until no more entities are added */
  e=set[setNr].line[0];
  do
  {
    sprintf( setname, "%s%d", specialset->cf,sum_c+1);
    delSet(setname);
    c1=pre_seta(setname,"i", 0);

    delSet("+buf");
    setNrbuf=pre_seta("+buf","l", line[e].name);
    completeSet_Lines(  setNrbuf, 0, elUsed, 1);                      // down, nodes from new elems in +buf
    for(i=0; i<set[setNrbuf].anz_p; i++)
    {
      seta(c1,"p", set[setNrbuf].pnt[i]);
    }

    seta(c1,"l", e);
    elUsed[e]=1; 
    do
    {
      sum1=set[c1].anz_p+set[c1].anz_l;

      delSet("+buf2");
      setTmp=pre_seta("+buf2", "i", 0 );
      completeSet_Lines( setNrbuf, setTmp, elUsed, 0);                  // up, only new elems in +buf2 (setTmp) based on nodes in +buf (setNrbuf)

      completeSet_Lines(  setTmp, 0, elUsed, 1);                      // down, nodes from new elems in +buf2 (setTmp)

      for(i=0; i<set[setTmp].anz_l; i++)
      {
        seta(c1,"l", set[setTmp].line[i]);
        elUsed[set[setTmp].line[i]]=1; 
      }

      delSet("+buf");
      setNrbuf=pre_seta("+buf","i", 0);
      for(i=0; i<set[setTmp].anz_p; i++)
      {
        seta(c1,"p", set[setTmp].pnt[i]);
        seta(setNrbuf,"p", set[setTmp].pnt[i]);
      }
      sum2=set[c1].anz_p+set[c1].anz_l;
    }while(sum1<sum2);

    //completeSet( setname, "do");                  // faces, nodes (to include also the additional midface-nodes for drawing)

    if(k>=SET_COLS) k=BAS_COLS-1;
    printf("set%d: %s col:%c\n",sum_c+1, setname, entity_k[k]);
    sprintf(buffer,"l %s %c", setname, entity_k[k++]);
    if(!sum_c) plot(buffer);
    else plus(buffer);

    sum_c++;
    e=0;
    for(i=0; i<set[setNr].anz_l; i++)
    {
      if(elUsed[set[setNr].line[i]]==0)
      {
        e=set[setNr].line[i];
        break;
      }
    }
  }while(e);

  free(elUsed);
  delSet("+buf");
  delSet("+buf2");

  printf (" ready\n");
}


void pre_contact( char *record)
{
  int i,j;
  int length, setNr, sum_c, c1, c2, mpcset[2];
  char setname[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH];
  double tol=0.;


  length=sscanf(record, "%s %lf %s", setname, &tol, action);
  if(length<3) strcpy(action,"null");

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", setname);
    return;
  }

  if (set[setNr].anz_f<2)
  {
    printf("ERROR: set:%s contains less than 2 faces. Contact between two faces not possible. Consider a <comp %s do> command.\n",set[setNr].name, set[setNr].name);
    return;
  }
  printf("Contact between faces of set:%s with tol:%lf\nwait for <ready>\n", set[setNr].name, tol);
  printf(" REMARK: Contact will only be detected if all nodes of adjacent element-faces are as close as the specified tolerance.\n");
  printf("         It will not be detected if only single nodes are within tolerance.\n");

  /*----- search the contact faces and write the eqations --------*/

  /* subdivide the faces into single unconnected sets of faces */
  /* return if not at least 2 indep meshes exist */
  sum_c=getMeshSections(setNr, face, node);
  if(sum_c < 2) 
  {
    printf("ERROR: Found only %d separate meshes. Contact not possible\n", sum_c);
    return;
  }
  printf(" found %d separate meshes\n", sum_c);


  /* search face-pairs of unconnected sets of faces */
  /* deside which side is the dep side */
  /* remove all dep-nodes which are not inside the ind-faces */

  /* ------- node-koordinaten berechnen und am ende wieder scalieren ------ */
  descalNodes ( anz->n, node, scale );

  //sum_c=0;
  for(i=1; i<sum_c; i++)
  {
    sprintf( setname, "%s%d", specialset->cf,i); 
    c1=getSetNr(setname);
    for(j=i; j<sum_c; j++)
    {
      sprintf( setname, "%s%d", specialset->cf,j+1); 
      c2=getSetNr(setname);

      if ((getFacePair( anz, c1, c2, e_enqire, face, node, tol, mpcset)==1)&&(action[0]=='e'))
      {
        /* write equations */
        /* write only dep-nodes which are not already used (1) */
        areampc(mpcset[0], mpcset[1], "abq", "areampc", "123", "c", 1, node, 1);
      }
    }
  }

  scalNodes ( anz->n, node, scale );
  printf("ready\n");
}



void pre_del( char *record)
{
  int  length, i, j, k, l, n,nn,m, s,jj;
  int setNr, nf=0, foundSubString, *lpnts, *lines;
  char type[MAX_LINE_LENGTH], **datum=NULL, **dat=NULL, *ori;
  char buffer[MAX_LINE_LENGTH];
  double  v01[3], ds;

  length= sword( record, type );
  for (i=0; i<MAX_PARAM_PER_RECORD; i++)
  {
    if((datum=(char **)realloc((char **)datum, (i+1)*sizeof(char *)))==NULL)
      printf("\n\n ERROR: realloc failure\n\n" );
    if((datum[i]=(char *)malloc( (MAX_LINE_LENGTH)*sizeof(char)))==NULL)
      printf("\n\n ERROR: realloc failure\n\n" );
    n =  sword( &record[length], datum[i] );
    if (n<1) break;
    length= length+n+1;
    operateAlias( datum[i], type );
  }

  /* check if wildcards (*) were used */
  k=0;
  for (n=0; n<i; n++)
  {
    length= strsplt( datum[n], '*', &dat);
    if ((length>0)&&(strstr(datum[n], "*") !=NULL))
    {
     if (compare(type, "se", 2) == 2 )
     {
      for(setNr=0; setNr<anz->sets; setNr++)
      {
        if( set[setNr].name == (char *)NULL ) continue;

        foundSubString=0;
        for(l=0; l<length; l++)
        {
          if(strstr(set[setNr].name, dat[l]) !=NULL)
          {
            foundSubString++;
            /* check if the first or the last char is no '*' then the substring[] must be at start or end */
            if(l==0) { if(datum[n][0]!='*')  { if(datum[n][0]!=set[setNr].name[0])  foundSubString--; }  }
            if(l==length-1) { if(datum[n][strlen(datum[n])-1]!='*') { if(datum[n][strlen(datum[n])-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
          }
        }

        if(foundSubString==length)
        {
          printf(" set:%s matches %s\n", set[setNr].name, datum[n]);
          if((datum=(char **)realloc((char **)datum, (i+k+1)*sizeof(char *)))==NULL)
            printf("\n\n ERROR: realloc failure\n\n" );
          if((datum[i+k]=(char *)malloc( (MAX_LINE_LENGTH)*sizeof(char)))==NULL)
            printf("\n\n ERROR: realloc failure\n\n" );
          strcpy(datum[i+k],set[setNr].name);
          k++;
        }
      }
     }
     else printf(" ERROR in %s, wildcards '*' are not supported for type:%s\n", datum[n], type);
    }
  }
  i+=k;
  if(printFlag) for (n=0; n<i; n++) printf("datum:'%s' \n", datum[n]);


  if (compare(type, "se0", 3) == 3 ) /* delete empty sets */
  {
    for (n=0; n<anz->sets; n++)
    {
      if((set[n].name != (char *)NULL )&&
         (!set[n].anz_v)&&
         (!set[n].anz_n)&&
         (!set[n].anz_e)&&
         (!set[n].anz_f)&&
         (!set[n].anz_p)&&
         (!set[n].anz_l)&&
         (!set[n].anz_c)&&
         (!set[n].anz_s)&&
         (!set[n].anz_b)&&
         (!set[n].anz_nurl)&&
         (!set[n].anz_nurs)&&
         (!set[n].anz_se)&&
	(!set[n].anz_sh)) { delSet(set[n].name); }
    }
    return;
  }
  else if ((compare(type, "se", 2) == 2 )||(compare(type, "sq", 2) == 2 ))
  {
    for (n=0; n<i; n++)
      delSet(datum[n]);
    return;
  }
  else if (compare(type, "sh", 2) == 2 )
  {
    for (n=0; n<i; n++)
    {
      j=getShapeNr(datum[n]);
      if(j>-1) delShape(1, &j);
      else printf(" WARNING: shape %s does not exist\n", datum[n]);
    }
    return;
  }
  else if (compare(type, "pic", 2) == 2 )
  {
    bgpicture=0;
    return;
  }
  else if(compare(type, "mesh", 2) == 2 )
  {
    /* delete the temporary sets which hold the dependend and independent nodes for incompatible elements */
    for(j=0; j<sum_equSets; j++) { delSet(set[depSet[j]].name); delSet(set[indSet[j]].name); }
    sum_equSets=0;

    /* delete the mesh */
    if(anz->n<1) return;
    if((i>0)&&(strlen(datum[i-1])>0)) 
    { 
      setNr=getSetNr(datum[i-1]);
      if(setNr<0)
      {
        printf(" ERROR: set |%s| does not exist\n",datum[i-1]);
        return;
      }
    }
    else setNr=0;
    if(printFlag) printf ("delete mesh defined in set:%s\n", set[setNr].name);
 
    if(setNr==getSetNr("all"))
    {
      for (j=0; j<anz->sets; j++ )
      {
        set[j].anz_n = 0;
        set[j].anz_e = 0;
        set[j].anz_f = 0;
      }
      for (j=0; j<anzGeo->psets; j++ )
      {
        set[pset[j].nr].anz_n=0;
        set[pset[j].nr].anz_e=0;
        set[pset[j].nr].anz_f=0;
      }

      for (j=0; j<anz->l; j++)
      {
        freeDataset(lcase, j);
      }
      free(lcase); lcase=NULL;

      for (j=0; j<anz->e; j++)
      {
        /* free space for the normal-vectors */
        if(e_enqire[e_enqire[j].nr].side!=NULL)
        {
          if (e_enqire[e_enqire[j].nr].type == 1)       nf=6;  /* HEXA8 */
          else if (e_enqire[e_enqire[j].nr].type == 2)  nf=6;  /* PENTA6 */
          else if (e_enqire[e_enqire[j].nr].type == 3)  nf=4;  /* TET4 */
          else if (e_enqire[e_enqire[j].nr].type == 4)  nf=48; /* HEXA20 */
          else if (e_enqire[e_enqire[j].nr].type == 5)  nf=48; /* PENTA15 */
          else if (e_enqire[e_enqire[j].nr].type == 6)  nf=16; /* TET10 */
          else if (e_enqire[e_enqire[j].nr].type == 7)  nf=1;  /* TRI3  */
          else if (e_enqire[e_enqire[j].nr].type == 8)  nf=4; /* TRI6  */
          else if (e_enqire[e_enqire[j].nr].type == 9)  nf=2; /* QUAD4 */
          else if (e_enqire[e_enqire[j].nr].type == 10) nf=8; /* QUAD8 */
          else if (e_enqire[e_enqire[j].nr].type == 11) nf=1; /* BEAM */
          else if (e_enqire[e_enqire[j].nr].type == 12) nf=1; /* BEAM3 */
          for(k=0; k<nf; k++) free(e_enqire[e_enqire[j].nr].side[k]);
          free(e_enqire[e_enqire[j].nr].side);
          e_enqire[e_enqire[j].nr].side=NULL;
        }
      }
      for (j=0; j<anz->f; j++)
      {
        if(face[j].side!=NULL)
        {
          if (face[j].type == 7)       nf=1;  /* TRI3  */
          else if (face[j].type == 8)  nf=4; /* TRI6  */
          else if (face[j].type == 9)  nf=2; /* QUAD4 */
          else if (face[j].type == 10) nf=8; /* QUAD8 */
          else if (face[j].type == 11) nf=1; /* BEAM */
          else if (face[j].type == 12) nf=1; /* BEAM3 */
          for(k=0; k<nf; k++) free(face[j].side[k]);
          free(face[j].side);
          face[j].side=NULL;
        }
      }
      if(anz->p)
      {
        for(i=0; i<anz->p; i++) free(anz->pheader[i]);
        free(anz->pheader);
        anz->p=0;
      }

      anz->n=0;
      anz->e=0;
      anz->g=0;
      anz->f=0;
      anz->l=0;
      anz->nmax=0;
      anz->nmin=0;
      anz->emax=0;
      anz->emin=0;
      anz->orignmax=anz->orign=anz->olc=0;

      for (j=0; j<anzGeo->p; j++)
      { free(point[j].nod);
        point[j].nod=NULL; 
        point[j].nn=-1; }
      for (j=0; j<anzGeo->l; j++)
      { free(line[j].nod); free(line[j].elem);
        line[j].nod=NULL; line[j].elem=NULL; 
        line[j].nn=-1; line[j].ne=-1; }
      for (j=0; j<anzGeo->s; j++)
      { surf[j].nn=-1; free(surf[j].nod); surf[j].nod=NULL;
        surf[j].ne=-1; free(surf[j].elem); surf[j].elem=NULL; }
      for (j=0; j<anzGeo->b; j++)
      { body[j].nn=-1; free(body[j].nod); body[j].nod=NULL;
        body[j].ne=-1; free(body[j].elem); body[j].elem=NULL; }
      if(drawMode!=4) plot("w all");
      updateDispLists();
      createNewMainMenu();
    }
    else
    {
      zap(specialset->zap);
      j=pre_seta( specialset->zap, "i", 0 );
      for (k=0; k<set[setNr].anz_n; k++) seta(j, "n",set[setNr].node[k]); 
      for (k=0; k<set[setNr].anz_e; k++) seta(j, "e",set[setNr].elem[k]); 
      if( zap(specialset->zap)==-1)
         printf (" ERROR in ZAP: set:%s does not exist\n",specialset->zap );
    }
    return;
  }
  else if ((compare(type, "l0", 2) == 2 )&&(strlen(type)==2))  /* delete lines of zero length */
  {
    setNr=getSetNr(datum[0]);
    if(setNr<0)
    {
      printf(" ERROR: set %s does not exist\n",datum[0]);
      return;
    }

    delSet("-mrgp");
    m=pre_seta("-mrgp","i", 0);

    for(i=0; i<set[setNr].anz_l; i++)
    {
      j=set[setNr].line[i];
      if( line[j].name == (char *)NULL ) continue;

      /* check the length of the line */
      v_result( &point[line[j].p1].px, &point[line[j].p2].px, v01 );
      ds=v_betrag( v01 );
      delSet(specialset->tmp);
      if(ds==0.)
      {
        printf("line:%s has zero length and will be deleted\n", line[j].name);
        jj=pre_seta(specialset->tmp,"l",line[j].name);
        completeSet(specialset->tmp,"do");
        sprintf(buffer,"p %s n",specialset->tmp);
        pre_merge(buffer);
        delSet(specialset->tmp);

        pre_seta( specialset->zap, "l", line[j].name );

        /* suche abhaengige lcmbs */
        for (jj=0; jj<anzGeo->c; jj++)
        {
          if( lcmb[jj].name != (char *)NULL )
          for (n=0; n<lcmb[jj].nl; n++)
          {
            if( j == lcmb[jj].l[n] )
            {
              if ((lines = (int *)malloc( (lcmb[jj].nl)*sizeof(int)) ) == NULL )
              { printf("ERROR: malloc failure\n"); return; }
              if ((ori = (char *)malloc( (lcmb[jj].nl)*sizeof(char)) ) == NULL )
              { printf("ERROR: malloc failure\n"); return; }
              l=0;              
              for (k=0; k<lcmb[jj].nl; k++)
              {
                if(lcmb[jj].l[k]!=j) { lines[l]= lcmb[jj].l[k]; ori[l++]=lcmb[jj].o[k]; }
                else if(!k) line[lcmb[jj].l[1]].div+=line[j].div;
                else line[lcmb[jj].l[k-1]].div+=line[j].div;
              }
              getNewName( buffer, "c" );
              l=lcmb_i( buffer, 0, lcmb[jj].nl-1, ori, lines );
              free(lines);
              free(ori);
              if (l >=0 )
	      {
                /* keep the original lcmb since the new lcmb was added to eventually open sets */
                free(lcmb[jj].l);
                free(lcmb[jj].o);
                lcmb[jj].l=lcmb[l].l;
                lcmb[jj].o=lcmb[l].o;
                lcmb[jj].nl=lcmb[l].nl;
                lcmb[jj].p1=lcmb[l].p1;
                lcmb[jj].p2=lcmb[l].p2;
                free(lcmb[l].name);
                lcmb[l].name = (char *)NULL;
	      }
              else
              {
                printf(" ERROR in del: lcmb could not be redefined\n");

                /* suche abhaengige surfs */
                for (s=0; s<anzGeo->s; s++)
                {
                  if( surf[s].name != (char *)NULL )
                  for (nn=0; nn<surf[s].nl; nn++)
                  {
                    if(( jj == surf[s].l[nn] )&&( surf[s].typ[nn] == 'c' ))
                    {
                      if ((lpnts = (int *)malloc( (surf[s].nl*2)*sizeof(int)) ) == NULL )
                      { printf("ERROR: malloc failure\n"); return; }
                      l=0;              
                      for (k=0; k<surf[s].nl; k++)
                      {
                        if(surf[s].l[k]!=jj)
                        {
                          surf[s].l[l]=surf[s].l[k];
                          surf[s].o[l]=surf[s].o[k];
                          surf[s].typ[l]=surf[s].typ[k];
                          l++;
                        }
		        else
		        {
                          if(surf[s].typ[k]=='l') { seta(m,"p", line[surf[s].l[k]].p1); seta(m,"p", line[surf[s].l[k]].p2); }
                          if(surf[s].typ[k]=='c') { seta(m,"p", lcmb[surf[s].l[k]].p1); seta(m,"p", lcmb[surf[s].l[k]].p2); }
			}
                      }
                      if (l <=0 )
                      {
                        printf(" WARNING: surf:%s is deleted\n", surf[s].name);
                        sprintf(buffer,"s %s", surf[s].name);
                        pre_del(buffer);
		      }
                      else surf[s].nl=l;
                      break;
                    }
                  }
                }
              }
              break;
            }
          }
        }
  
        /* suche abhaengige surfs */
        for (jj=0; jj<anzGeo->s; jj++)
        {
          if( surf[jj].name != (char *)NULL )
          for (n=0; n<surf[jj].nl; n++)
          {
            if(( j == surf[jj].l[n] )&&( surf[jj].typ[n] == 'l' ))
            {
              if ((lpnts = (int *)malloc( (surf[jj].nl*2)*sizeof(int)) ) == NULL )
              { printf("ERROR: malloc failure\n"); return; }
              l=0;              
              for (k=0; k<surf[jj].nl; k++)
              {
                if(surf[jj].l[k]!=j)
                {
                  surf[jj].l[l]=surf[jj].l[k];
                  surf[jj].o[l]=surf[jj].o[k];
                  surf[jj].typ[l]=surf[jj].typ[k];
                  l++;
                }
	        else
	        {
                  if(surf[jj].typ[k]=='l') { seta(m,"p", line[surf[jj].l[k]].p1); seta(m,"p", line[surf[jj].l[k]].p2); }
                  if(surf[jj].typ[k]=='c') { seta(m,"p", lcmb[surf[jj].l[k]].p1); seta(m,"p", lcmb[surf[jj].l[k]].p2); }
		}
              }
              if (l <=0 )
              {
                printf(" WARNING: surf:%s is deleted\n", surf[jj].name);
                sprintf(buffer,"s %s", surf[jj].name);
                pre_del(buffer);
	      }
              else surf[jj].nl=l;
              break;
            }
          }
        }
      }

    }
    if( zap(specialset->zap)==-1)
          printf (" ERROR in ZAP: set:%s does not exist\n",specialset->zap ); 
    pre_merge("p -mrgp");
    delSet("-mergp");
    return;
  }
  else if (compare(type, "t", 1) == 1 )
  {
    setNr=getSetNr(datum[0]);
    if(setNr<0)
    {
      printf(" ERROR: set |%s| does not exist\n",set[setNr].name);
      return;
    }
    for(i=0; i<set[setNr].anz_n; i++)
    {
      for(j=0; j<anz->t; j++)
        if(ntext[j].node_nr==set[setNr].node[i]) ntext[j].node_nr=0;
    }
    return;
  }
  else if (((compare(type, "n", 1) == 1 )||
      (compare(type, "e", 1) == 1 )||
      (compare(type, "p", 1) == 1 )||
      (compare(type, "l", 1) == 1 )||
      (compare(type, "c", 1) == 1 )||
      (compare(type, "s", 1) == 1 )||
      (compare(type, "v", 1) == 1 )||
      (compare(type, "b", 1) == 1 )||
      (compare(type, "S", 1) == 1 ))&&(strlen(type)==1)) 
  {
    for (n=0; n<i; n++) pre_seta( specialset->zap, type, datum[n] );
    if( zap(specialset->zap)==-1)
          printf (" ERROR in ZAP: set:%s does not exist\n",specialset->zap );
    return;
  }
  else printf(" ERROR: type %s not known\n", type);
}


void pre_bia( char *record)
{
  int  j=0, nr, setNr;
  char setname[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH];
  double factor=1., value=1.;
  double bias,div;
  int bias_fbd;     /* bias * 10 to gain one decimal digit (bias 10 == 100) */

  sscanf( record, "%s%s%lf%lf", setname, type, &factor, &value );

  if ((type[0]=='M')||(type[0]=='m')||(type[0]=='D')||(type[0]=='d') )
  {
    if ((type[0]=='D')||(type[0]=='d') ) factor=1./factor;
    operateAlias( setname, "se" );
    nr=getSetNr( setname );
    if(nr<0)
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
    for (j=0; j<set[nr].anz_l; j++)
    {
      div=line[set[nr].line[j]].div;
      bias=line[set[nr].line[j]].bias;
      if(bias<1.)
      {
        bias_fbd= (pow(1./bias,(div-1.))*-10.)-.5;
        bias_fbd*=factor;
        if(bias_fbd>-10) line[set[nr].line[j]].bias=1.;
        else line[set[nr].line[j]].bias= 1./pow(((double)bias_fbd*-0.1), (1./((double)div-1.)));
      }
      else if(bias>1.)
      {
        bias_fbd= (pow(bias,(div-1.))*10)+.5;
        bias_fbd*=factor;
        if(bias_fbd<10) line[set[nr].line[j]].bias=1.;
        else line[set[nr].line[j]].bias=pow(((double)bias_fbd*0.1), (1./((double)div-1.)));
      }
    }
  }
  else if((type[0]>='1')&&(type[0]<='9'))
  {
    operateAlias( setname, "l" );
    nr=getLineNr( setname );

    operateAlias( setname, "se" );
    setNr=getSetNr( setname );

    /* its a line */
    if(nr>-1)
    {
      div=line[nr].div;
      bias=atof( &type[0])*factor;
      if(div>1)
      {
        if(bias<0) bias= 1./pow((bias*-1), (1./((double)div-1.)));
        else       bias=pow(bias, (1./((double)div-1.)));
      }
      else bias=1.;
      line[nr].bias=bias;
      repLine(nr);
    }
    /* its a set */
    else if(setNr>-1)
    {
      for (j=0; j<set[setNr].anz_l; j++)
      {
        div=line[set[setNr].line[j]].div;
        if(div>1)
        {
          bias=atof(type)*factor;
          if(bias<0) bias= 1./pow((bias*-1), (1./((double)div-1.)));
          else       bias=pow(bias, (1./((double)div-1.)));
        }
        else bias=1.;
	printf("bias:%f factor:%f\n", bias,factor);
        line[set[setNr].line[j]].bias=bias;
      }
      for (j=0; j<set[setNr].anz_l; j++) repLine(set[setNr].line[j]);
    }
    else
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
  }
  else
  {
    errMsg(" ERROR: command:|%s| not recognized\n", record );
  }
  return;
}


void pre_div( char *record)
{
  int  length,j=0, nr, setNr;
  char setname[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH];
  double factor=1., value=1., bias=1., length_ratio=ELEM_LENGTH_RATIO;
  int bias_fbd,div;

  length= sscanf( record, "%s%s%lf%lf%lf", setname, type, &factor, &value, &length_ratio);

  if(length<1)
  {
    printf(" default:%d\n auto mode: lmax:%e angle:%.1f ratio:%.3f\n",ddiv,gtol*GTOL_NODE_DIST,acos(GTOL_COS_A)*180./PI, length_ratio);
    return;
  }

  if((length==1)&&(atoi(setname)>0))
  {
    printf("DDIV %s\n",setname);
    sprintf(type,"DDIV %s",setname);
    pre_proc(type);
  }
  else if ((length>2)||(compare(type, "auto", 1) == 1 ))
  {
    /* its a set */
    operateAlias( setname, "se" );
    nr=getSetNr( setname );
    if(nr<0)
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
    if ((type[0]=='A')||(type[0]=='a') )
    {
      if(length_ratio>1.)
      {
        printf(" ERROR: element length ratio must be lower than 1 but is:%f\n", length_ratio  );
        return;
      }
      if(length==2) { value=GTOL_COS_A; factor=gtol*GTOL_NODE_DIST/scale->w; }
      else if(length==3) { value=GTOL_COS_A; factor/=scale->w; }
      else  { value=cos(value*PI/180.); factor/=scale->w; }
      for (j=0; j<set[nr].anz_l; j++)
      {
        calcLineDiv(line, set[nr].line[j], value, factor, factor*length_ratio  ); 
      }
    }
    else if ((type[0]=='M')||(type[0]=='m') )
    {
      for (j=0; j<set[nr].anz_l; j++)
      {
        if((line[set[nr].line[j]].bias!=1.)&&(line[set[nr].line[j]].div>1))
	{
          if(line[set[nr].line[j]].bias<1.)
	  {
            bias_fbd= (int)((pow(1./line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*-10.) -0.5) * value;
	  }
          else
	  {
            bias_fbd= (int)((pow(line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*10.) +0.5) * value;
	  }
          line[set[nr].line[j]].div*=factor;
          if(bias_fbd*bias_fbd<10.) line[set[nr].line[j]].bias=1.;
          else
          {
            if(bias_fbd<0) line[set[nr].line[j]].bias= 1./pow((bias_fbd*-.1), (1./((double)line[set[nr].line[j]].div-1.)));
            else       line[set[nr].line[j]].bias=pow((bias_fbd*.1), (1./((double)line[set[nr].line[j]].div-1.)));
          }
        }
        else line[set[nr].line[j]].div*=factor;
      }
    }
    else if ((type[0]=='D')||(type[0]=='d') )
    {
      for (j=0; j<set[nr].anz_l; j++)
      {
        if((line[set[nr].line[j]].bias!=1.)&&(line[set[nr].line[j]].div>1))
	{
          if(line[set[nr].line[j]].bias<1.)
	  {
            bias_fbd= (int)((pow(1./line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*-10.) -0.5) / value;
	  }
          else
	  {
            bias_fbd= (int)((pow(line[set[nr].line[j]].bias,(line[set[nr].line[j]].div-1.))*10.) +0.5) / value;
	  }
          line[set[nr].line[j]].div/=factor;
          if(bias_fbd*bias_fbd<10.) line[set[nr].line[j]].bias=1.;
          else
          {
            if(bias_fbd<0) line[set[nr].line[j]].bias= 1./pow((bias_fbd*-.1), (1./((double)line[set[nr].line[j]].div-1.)));
            else       line[set[nr].line[j]].bias=pow((bias_fbd*.1), (1./((double)line[set[nr].line[j]].div-1.)));
          }
        }
        else line[set[nr].line[j]].div/=factor;
        if(line[set[nr].line[j]].div<1) line[set[nr].line[j]].div=1;
      }
    }
    else if (length==2)
    {
      div=atoi(type);
      splitBiasDiv(&div, &bias);
      line[set[nr].line[j]].div=div;
      line[set[nr].line[j]].bias=bias;
    }
    else printf(" ERROR: type %s not known\n", type);
  
    for (j=0; j<set[nr].anz_l; j++) repLine(set[nr].line[j]);
  }
  else if((length==2)&&(type[0]>='1')&&(type[0]<='9'))
  {
    operateAlias( setname, "l" );
    nr=getLineNr( setname );

    operateAlias( setname, "se" );
    setNr=getSetNr( setname );

    /* its a line */
    if(nr>-1)
    {
      div=atoi(type);
      //splitBiasDiv(&div, &bias);

      line[nr].div=div;
      //line[nr].bias=bias;
      repLine(nr);
    }
    /* its a set */
    else if(setNr>-1)
    {
      div=atoi(type);
      //splitBiasDiv(&div, &bias);
      for (j=0; j<set[setNr].anz_l; j++)
      {
        line[set[setNr].line[j]].div=div;
        //line[set[setNr].line[j]].bias=bias;
      }
      for (j=0; j<set[setNr].anz_l; j++) repLine(set[setNr].line[j]);
    }
    else
    {
      errMsg(" ERROR: %s is undefined\n", setname );
      return;
    }
  }
  else
  {
    errMsg(" ERROR: command:|%s| not recognized\n", record );
  }
  return;
}


void pre_move( char *record)
{
  int  length, i,j,k,p, setNr, nr;
  char setname[MAX_LINE_LENGTH];
  char type[MAX_LINE_LENGTH];
  Nodes *bufn;
  char addDispFlagLocal=0;

  length = sword( record, setname );
  sword( &record[length], type );

  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }
  nr=setNr;

  if (set[nr].anz_p>set[nr].anz_n) i=set[nr].anz_p;
  else                             i=set[nr].anz_n;

  if ( (bufn = (Nodes *)malloc( (i+1) * sizeof(Nodes))) == NULL )
  {  printf("\n\n ERROR: malloc failed in pre_move(): i=%d\n\n", i) ; return; }


  if(set[nr].anz_p>0)
  {
    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      if (p==-1)
      {
        errMsg(" ERROR: Point-nr:%d is undefined\n", set[nr].pnt[i] );
        return;
      }
      bufn[i].nx=point[p].px*scale->w+scale->x;
      bufn[i].ny=point[p].py*scale->w+scale->y;
      bufn[i].nz=point[p].pz*scale->w+scale->z;
    }
    if(transform( &record[length], set[nr].anz_p, bufn )<0) return;

    for (i=0; i<set[nr].anz_p; i++)
    {
      p=set[nr].pnt[i];
      point[p].px= (bufn[i].nx-scale->x)/scale->w;
      point[p].py= (bufn[i].ny-scale->y)/scale->w;
      point[p].pz= (bufn[i].nz-scale->z)/scale->w;
    }
  }
  if(set[nr].anz_n>0)
  {
    /* when node coordinates were changed to the deformed ones then switch back before they are copied and then switch again */ 
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      // remember to switch back
      addDispFlagLocal=2;
    }

    for (i=0; i<set[nr].anz_n; i++)
    {
      bufn[i].nx=node[set[nr].node[i]].nx* scale->w+scale->x;
      bufn[i].ny=node[set[nr].node[i]].ny* scale->w+scale->y;
      bufn[i].nz=node[set[nr].node[i]].nz* scale->w+scale->z;
    }
    if ( transform( &record[length], set[nr].anz_n, bufn) <0) return;
    for (i=0; i<set[nr].anz_n; i++)
    {
      node[set[nr].node[i]].nx= (bufn[i].nx-scale->x)/scale->w;
      node[set[nr].node[i]].ny= (bufn[i].ny-scale->y)/scale->w;
      node[set[nr].node[i]].nz= (bufn[i].nz-scale->z)/scale->w;
    }

    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
  }

  /* the additional nodes for the HE20 etc. (then HE26) elements need a new pos. too  */
  if(anz->e>0)
  {
    elemChecker( anz->emax+1, node, e_enqire);
    adjustDrawNodes(0);
    getElemNormalen( e_enqire, node, anz->e );
    makeSurfaces();
    updateDispLists();
  }

  /* dependent higher entities need new label-positions, achived with orient */
  /* create a set with the dependent higher entities to be oriented  */

  /* add all points to special set _ORI  */
  if(set[nr].anz_p>0)
  {
    if( (i=pre_seta( specialset->ori, "i", 0 )) <0 ) return;
    for (k=0; k<set[nr].anz_p; k++)
    {
      p=set[nr].pnt[k];
      seta( i, "p", p );
    }
    completeSet( specialset->ori, "up");
    orientSet( specialset->ori );

    /*  recalculate the line-shapes    */
    for (j=0; j<set[i].anz_l; j++) repLine(set[i].line[j]);

    /* clear special set  */
    delSet(specialset->ori);
  }
  free (bufn);
  if(( compare(type,"sca",3) == 3 )||( compare(type,"SCA",3) == 3 )) frame(); 
}


void free_proj(double *nx, double *nrad, int *nmissed)
{
  free (nx);
  free (nrad);
  free (nmissed);
  delSet(specialset->dep );
  delSet(specialset->ind );
}



void pre_proc( char *record)
{
  int  i;
  char action[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];

  sscanf( record,"%s %s", action, param);

  for(i=0; i<strlen(action); i++) action[i]=toupper(action[i]);
  if (compareStrings(action, "DDIV")>0)
  {
    ddiv=atoi(param);
    splitBiasDiv(&ddiv, &dbias);
  }
}


/* l line, l_nr two new lines, ps_nr split-point */
int splitLine(int l, int *l_nr, int ps_nr, double ps_lbez)
{
  int  i,k, setNr;
  int  setNr0, setNr1;
  char name[MAX_LINE_LENGTH], trk[2][MAX_LINE_LENGTH];

  int p1_nr, p2_nr;
  char linbuf[MAX_LINE_LENGTH];
  double lbez, div;
  double p0[3], p1[3], p01[3];

  static double *pset_dl=NULL;

  double ltot,l1;
  int    div1,div2;


  /*  erzeuge den trk der neuen linien */
  if (line[l].typ=='a')
  {
    strcpy( trk[0], point[line[l].trk].name );
    strcpy( trk[1], point[line[l].trk].name );
  }
  else if (line[l].typ=='s')
  {
    /* suche alle punkte des trk die vor dem schnittpunkt liegen und ordne sie einem */
    /* neuen set zu und loesche sie aus dem bestehenden trk */
  
    /* erzeuge zwei neue sets fuer die stuetzpunkte der zwei neuen linien */
    getNewName( trk[0], "se" );
    setNr0=pre_seta( trk[0], "is", 0);
  
    getNewName( trk[1], "se" );
    setNr1=pre_seta( trk[1], "is", 0);
  
    /* determine the length of the spline, accumulate all dists between points */
    /* look if the sequence is ordered in the same way as the line */
    setNr=line[l].trk;
    lbez=0.;
   
    if ( ( pset_dl= (double *)realloc( (double *)pset_dl, (set[setNr].anz_p+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed: splitLine\n\n" ); 
    pset_dl[0]=pset_dl[set[setNr].anz_p-1]=0.;
    if( line[l].p1==set[setNr].pnt[0] )
    {
      p0[0]=point[set[setNr].pnt[0]].px;
      p0[1]=point[set[setNr].pnt[0]].py;
      p0[2]=point[set[setNr].pnt[0]].pz;   
      for (i=1; i<set[setNr].anz_p; i++)
      {
        p1[0]=point[set[setNr].pnt[i]].px;
        p1[1]=point[set[setNr].pnt[i]].py;
        p1[2]=point[set[setNr].pnt[i]].pz;
        v_result( p0, p1, p01  );
        p0[0]=p1[0];
        p0[1]=p1[1];
        p0[2]=p1[2];
        lbez+=v_betrag( p01  );
        pset_dl[i]=lbez;
      }
    }
    else
    {
      p0[0]=point[set[setNr].pnt[set[setNr].anz_p-1]].px;
      p0[1]=point[set[setNr].pnt[set[setNr].anz_p-1]].py;
      p0[2]=point[set[setNr].pnt[set[setNr].anz_p-1]].pz;   
      for (i=1; i<set[setNr].anz_p; i++)
      {
        p1[0]=point[set[setNr].pnt[set[setNr].anz_p-1-i]].px;
        p1[1]=point[set[setNr].pnt[set[setNr].anz_p-1-i]].py;
        p1[2]=point[set[setNr].pnt[set[setNr].anz_p-1-i]].pz;
        v_result( p0, p1, p01  );
        p0[0]=p1[0];
        p0[1]=p1[1];
        p0[2]=p1[2];
        lbez+=v_betrag( p01  );
        pset_dl[set[setNr].anz_p-1-i]=lbez;
      }
    }
    ps_lbez*=lbez;
  
    k=0;
    if( line[l].p1==set[setNr].pnt[0] )
    {
      for (i=0; i<set[setNr].anz_p; i++)
      { 
        /* printf("+ dl:%lf ls:%lf p:%s\n", pset_dl[i], ps_lbez, point[set[setNr].pnt[i]].name); */
        if(pset_dl[i]<ps_lbez) seta( setNr0, "ps", set[setNr].pnt[i] );
        else
        {
          if(!k)
          {
            /* use the close track-points only if the distance to ps_nr is significant */
            v_result( &point[set[setNr].pnt[i-1]].px, &point[ps_nr].px, p01  );
            if(v_betrag( p01 )<gtol) set[setNr0].anz_p--;

            seta( setNr0, "ps", ps_nr ); 
            seta( setNr1, "ps", ps_nr ); k++;

            v_result( &point[set[setNr].pnt[i]].px, &point[ps_nr].px, p01  );
            if(v_betrag( p01 )>gtol) seta( setNr1, "ps", set[setNr].pnt[i] );   
          } 
          else seta( setNr1, "ps", set[setNr].pnt[i] );
        }
      }
    }
    else
    {
      for (i=set[setNr].anz_p-1; i>=0; i--)
      { 
        /* printf("- dl:%lf ls:%lf p:%s\n", pset_dl[i], ps_lbez, point[set[setNr].pnt[i]].name); */
        if(pset_dl[i]<ps_lbez) seta( setNr0, "ps", set[setNr].pnt[i] );
        else
        {
          if(!k)
          {
            /* use the close track-points only if the distance to ps_nr is significant */
            v_result( &point[set[setNr].pnt[i+1]].px, &point[ps_nr].px, p01  );
            if(v_betrag( p01 )<gtol) set[setNr0].anz_p--;

            seta( setNr0, "ps", ps_nr ); 
            seta( setNr1, "ps", ps_nr ); k++;

            v_result( &point[set[setNr].pnt[i]].px, &point[ps_nr].px, p01  );
            if(v_betrag( p01 )>gtol) seta( setNr1, "ps", set[setNr].pnt[i] );
          } 
          else seta( setNr1, "ps", set[setNr].pnt[i] );
        }
      }
    }
  }
  else
  {
    strcpy(trk[0], " ");
    strcpy(trk[1], " ");
  }

  /* sichere den liniennamen bevor der pointer durch realloc in line_ geaendert wird  */
  strcpy( linbuf, line[l].name);
  
  /* erzeuge zwei neue linien  */
  p1_nr = line[l].p1;
  p2_nr = line[l].p2;

  div=1;
  getNewName( name, "l" );
  if(printFlag) printf(" create line:%s %s %s %s %lf\n", name, point[p1_nr].name, point[ps_nr].name, trk[0], div );
  line_( name, point[p1_nr].name, point[ps_nr].name, trk[0], div, 1 );
  l_nr[0]=getLineNr(name);
  getNewName( name, "l" );
  if(printFlag) printf(" create line:%s %s %s %s %lf\n", name, point[ps_nr].name, point[p2_nr].name, trk[1], div );
  line_( name, point[ps_nr].name, point[p2_nr].name, trk[1], div, 1 );
  l_nr[1]=getLineNr(name);

  /* calc new div */
  if (line[l].div>1)
  {
    ltot=calcLineLength(l);
    l1=calcLineLength(l_nr[0]);
    div1=line[l].div/ltot*l1;
    div2=line[l].div-div1;
    if(printFlag)  printf("1 ltot:%lf l1:%lf ldiv:%d div1:%d div2:%d\n", ltot,l1,line[l].div,div1,div2); 

    if(div1<1) { div1=1; div2=line[l].div-div1; }
    if(div2<1) { div2=1; div1=line[l].div-div2; }
    line[l_nr[0]].div=div1;
    line[l_nr[1]].div=div2;
    if(printFlag)  printf("2 ltot:%lf l1:%lf ldiv:%d div1:%d div2:%d\n", ltot,l1,line[l].div,div1,div2); 
    repLine(l_nr[0]);
    repLine(l_nr[1]);
  }
  return(1);
}


int genSplitTrias(int trgtNr, Tri **ptr_tri, int scalFlag )
{
  int i,j,n,s;
  int sum_tri=0, ss;
  Tri *tri;

  tri=*ptr_tri;

  for (ss=0; ss<set[trgtNr].anz_sh; ss++)
  {
    s=set[trgtNr].shp[ss];
    if(printFlag) printf("shape:%s\n", shape[s].name);

    /* go over all shapes */
    n=0;
    while((shape[s].npgn-n))
    {
      /* alloc a new tri */
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
      {
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      }

      /* requires that only triangles are used for the interiour description of the shape */
      /* this is regarded by the adjustFeedBack() routine */
      //n+=5; /* jump over the leading bytes */
      tri[sum_tri].p1[0]=shape[s].pgn[n++];
      tri[sum_tri].p1[1]=shape[s].pgn[n++];
      tri[sum_tri].p1[2]=shape[s].pgn[n++];
      tri[sum_tri].p2[0]=shape[s].pgn[n++];
      tri[sum_tri].p2[1]=shape[s].pgn[n++];
      tri[sum_tri].p2[2]=shape[s].pgn[n++];
      tri[sum_tri].p3[0]=shape[s].pgn[n++];
      tri[sum_tri].p3[1]=shape[s].pgn[n++];
      tri[sum_tri].p3[2]=shape[s].pgn[n++];
      tri[sum_tri].fnr= -1;
      sum_tri++;
    }
  }
  /* go over all surfs */
  for (ss=0; ss<set[trgtNr].anz_s; ss++)
  {
    s=set[trgtNr].surf[ss];

    /* go over all tri */
    n=0;
    while((surf[s].npgn-n))
    {
      /* alloc a new tri */
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
      {
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      }

      /* requires that only triangles are used for the interiour description of the surface */
      /* this is regarded by the adjustFeedBack() routine */
      n+=5; /* jump over the leading bytes */
      tri[sum_tri].p1[0]=surf[s].pgn[n++];
      tri[sum_tri].p1[1]=surf[s].pgn[n++];
      tri[sum_tri].p1[2]=surf[s].pgn[n++];
      tri[sum_tri].p2[0]=surf[s].pgn[n++];
      tri[sum_tri].p2[1]=surf[s].pgn[n++];
      tri[sum_tri].p2[2]=surf[s].pgn[n++];
      tri[sum_tri].p3[0]=surf[s].pgn[n++];
      tri[sum_tri].p3[1]=surf[s].pgn[n++];
      tri[sum_tri].p3[2]=surf[s].pgn[n++];
      tri[sum_tri].fnr= -1;
      sum_tri++;
    }
  }
  for (ss=0; ss<set[trgtNr].anz_f; ss++)
  {
    i=set[trgtNr].face[ss];

    if (face[i].type == 7)  /* TRI3  */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+2) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
    }
    else if (face[i].type == 8)  /* TRI6  */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+6) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[5]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[4]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
    }
    else if (face[i].type == 9)  /* QUAD4 */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+4) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].fnr= i;
      sum_tri++;
    }
    else if (face[i].type == 10) /* QUAD8 */
    {
      if ( (tri = (Tri *)realloc( (Tri *)tri, (sum_tri+8) * sizeof(Tri))) == NULL )
        errMsg("\nERROR: realloc failed in genSplitTrias() \n\n");
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[0]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[4]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[4]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[4]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[4]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[1]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[1]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[1]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[1]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[5]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[5]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[5]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[5]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[2]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[2]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[2]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[2]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[6]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[6]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[6]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[6]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[6]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[6]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[3]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[3]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[3]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[3]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[7]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[7]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[7]].nz;
      sum_tri++;
      tri[sum_tri].p1[0]=node[face[i].nod[8]].nx;
      tri[sum_tri].p1[1]=node[face[i].nod[8]].ny;
      tri[sum_tri].p1[2]=node[face[i].nod[8]].nz;
      tri[sum_tri].p2[0]=node[face[i].nod[7]].nx;
      tri[sum_tri].p2[1]=node[face[i].nod[7]].ny;
      tri[sum_tri].p2[2]=node[face[i].nod[7]].nz;
      tri[sum_tri].p3[0]=node[face[i].nod[0]].nx;
      tri[sum_tri].p3[1]=node[face[i].nod[0]].ny;
      tri[sum_tri].p3[2]=node[face[i].nod[0]].nz;
      sum_tri++;
    }
  }
  for(j=0; j<sum_tri; j++)
  {
    if(scalFlag)
    {
      tri[j].p1[0]= tri[j].p1[0]*scale->w+scale->x; tri[j].p1[1]=tri[j].p1[1]* scale->w+scale->y; tri[j].p1[2]= tri[j].p1[2]*scale->w+scale->z;
      tri[j].p2[0]= tri[j].p2[0]*scale->w+scale->x; tri[j].p2[1]=tri[j].p2[1]* scale->w+scale->y; tri[j].p2[2]= tri[j].p2[2]*scale->w+scale->z;
      tri[j].p3[0]= tri[j].p3[0]*scale->w+scale->x; tri[j].p3[1]=tri[j].p3[1]* scale->w+scale->y; tri[j].p3[2]= tri[j].p3[2]*scale->w+scale->z;
    }

    /* "cg" of tri       */
    tri[j].cg[0]=(tri[j].p1[0]+tri[j].p2[0]+tri[j].p3[0])/3;
    tri[j].cg[1]=(tri[j].p1[1]+tri[j].p2[1]+tri[j].p3[1])/3;
    tri[j].cg[2]=(tri[j].p1[2]+tri[j].p2[2]+tri[j].p3[2])/3;
  }

  *ptr_tri=tri;
  return(sum_tri);
}



void pre_split( char *record)
{
  int  c,e,i,j=0,k,n,nn,m,p,ll,l,d1,d2,ss,s, setNr, trgtNr, mode, ibuf=0, orient=0;
  char setname[MAX_LINE_LENGTH], targetname[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH];
  double pl[2][3],ps[3][3], pc[3], psplt[10][3], vproj[3], dist=0., dist2=0., distn=0., lp0p1, lps, ltot;
  double pt1[3], pt2[3], pt3[3], vs[3];
  double cg_edge[3][3];

  int p1_nr, p2_nr, ps_nr, l_nr[2], sum_psplt, setSplit;
  static int *lbuffer=NULL, sum_l=0;

  Rsort *rsort=NULL;
  Tri *tri=NULL;
  int sum_tri=0;
  float dx,dy,dz;

  int   ii, jj=0, ns, nm, nc=0, n1, n2, ee, ec, en1, en2, nn1, nn2, setdep, setind, nunspl[6][3];
  double  vn_tri[12][3], vn_av[3];
  int  set_efail, set_eorig=0, set_etop=0, set_ebot=0, set_ntop=0, set_nbot=0, ecounter=0;

  int  nodseq_te4[]={0,1,1,2,2,0, 0,3,1,3,2,3};
  int  nodseq_te10[]={0,4,1,1,5,2,2,6,0, 0,7,3,1,8,3,2,9,3};
  int  nodseq_pe6[]={0,1,1,2,2,0, 3,4,4,5,5,3, 0,3,1,4,2,5};
  int  nodseq_pe15[]={0,6,1,1,7,2,2,8,0, 3,12,4,4,13,5,5,14,3, 0,9,3,1,10,4,2,11,5};
  int  edge[20], nnew[20], ncpy[20], nel[20], nbuf[20], nref[20][2];
  double v12[3], v13[3], v15[3], v15n[3], vn[3];
  double bv15=0., bvn, bv15n, bgrenz, ltol;
  int  ncollapsed[27];

  int     n_closest_nodes;
  double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL, near_node[N_CLOSEST_TRI];

  typedef struct {
      int sum, *n2, *nm, *nc;
  }N1nm;
  N1nm *n1ns;
  N1nm *n1nm;

  sscanf( record,"%s %s %s", setname, targetname, param );

  delSet(specialset->dep );
  delSet(specialset->ind );

  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }

  /* first assume the target is a single shape */
  operateAlias( targetname, "sh" );
  trgtNr=getShapeNr( targetname );
  if (trgtNr>-1)
  {
    if( (setind=pre_seta( specialset->ind, "sh", targetname )) <0 ) return;
    trgtNr=-1;
  }
  else
  {
    operateAlias( targetname, "se" );
    trgtNr=getSetNr( targetname );
    if (trgtNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", targetname );
      return;
    }

    /* cycle through all entities of targetNr and add them to the special set  */
    /* cyrcle through all bodies and add  */
    if( (setind=pre_seta( specialset->ind, "i", 0 )) <0 ) return;
    for (i=0; i<set[trgtNr].anz_s; i++)
    {
      seta( setind, "s", set[trgtNr].surf[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
    for (i=0; i<set[trgtNr].anz_l; i++)
    {
      seta( setind, "l", set[trgtNr].line[i] );
    }
    for (i=0; i<set[trgtNr].anz_f; i++)
    {
      seta( setind, "f", set[trgtNr].face[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
  }

  /* second cycle through all entities and add lower ones to the special set  */
  completeSet( specialset->ind, "do") ;

  /* cycle through all entities of setNr and add them to the special set  */
  /* cyrcle through all bodys and add  */
  if( (setdep=pre_seta( specialset->dep, "i", 0 )) <0 ) return;
  for (i=0; i<set[setNr].anz_b; i++)
  {
    seta( setdep, "b", set[setNr].body[i] );
  }
  /* cyrcle through all surfs and add  */
  for (i=0; i<set[setNr].anz_s; i++)
  {
    seta( setdep, "s", set[setNr].surf[i] );
  }
  /* cyrcle through all lcmbs and add  */
  for (i=0; i<set[setNr].anz_c; i++)
  {
    seta( setdep, "c", set[setNr].lcmb[i] );
  }
  /* cyrcle through all lines and add  */
  for (i=0; i<set[setNr].anz_l; i++)
  {
    seta( setdep, "l", set[setNr].line[i] );
  }
  for (i=0; i<set[setNr].anz_p; i++)
  {
    seta( setdep, "p", set[setNr].pnt[i] );
  }
  for (i=0; i<set[setNr].anz_e; i++)
  {
    seta( setdep, "e", set[setNr].elem[i] );
  }
  /* second cycle through all entities and add lower ones  to the special set  */
  completeSet( specialset->dep, "do") ;

  if (set[setNr].anz_n==0) set[setdep].anz_n=set[setNr].anz_n;
  setNr=setdep;
  trgtNr=setind;

  /* fill all unfilled surfaces, do it in any case because points could have been moved! */
  /* repShape will change *scale and therefore must be executed first */
  repShape(trgtNr);
  for (i=0; i<set[setNr].anz_l; i++) repLine(set[setNr].line[i]);
  for (j=0; j<set[trgtNr].anz_s; j++)
  {
    //orientSet( set[trgtNr].name );
    if(surf[set[trgtNr].surf[j]].sh>-1) { repNurs(surf[set[trgtNr].surf[j]].sh); untrimNurs(surf[set[trgtNr].surf[j]].sh); }
    repSurf(set[trgtNr].surf[j],1);
  }
  for (j=0; j<set[setNr].anz_s; j++)
  {
    repSurf(set[setNr].surf[j],1);
  }

  /* generate splitting triangles */
  sum_tri=genSplitTrias(trgtNr, &tri, 0);
  
  /*
  printf("sum_tri:%d\n",sum_tri);
  */

  ltol=LTOL;

  /* 0) split all elems */
  if(set[setdep].anz_e)
  {
    set_efail=pre_seta("+efail","i",0);
    if(param[0]=='e') set_eorig=pre_seta("+eorig","i",0);
    else
    {
      set_etop=pre_seta("+etop","i",0);
      set_ebot=pre_seta("+ebot","i",0);
      set_ntop=pre_seta("+ntop","i",0);
      set_nbot=pre_seta("+nbot","i",0);
    }

    /* free the additional midside-nodes for higher order elements */
    for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
    anz->n= anz->orign;
    anz->nmax=anz->orignmax;

    /* create a table for all nodes which points to already created split nodes */
    if ( (n1ns = (N1nm *)malloc( (anz->nmax+1) * sizeof(N1nm))) == NULL )
    { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
    for (i=0; i<=anz->nmax; i++) n1ns[i].sum=0;
    for (i=0; i<=anz->nmax; i++) n1ns[i].n2=n1ns[i].nm=n1ns[i].nc=NULL;

    /* create a table for all nodes which points to already created midside nodes */
    if ( (n1nm = (N1nm *)malloc( (anz->nmax+1) * sizeof(N1nm))) == NULL )
    { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
    for (i=0; i<=anz->nmax; i++) n1nm[i].sum=0;
    for (i=0; i<=anz->nmax; i++) n1nm[i].n2=n1nm[i].nm=n1nm[i].nc=NULL;

    /* stelle daten fuer near3d bereit */
    if((int)N_CLOSEST_TRI<sum_tri) n_closest_nodes=(int)N_CLOSEST_TRI; else n_closest_nodes=sum_tri;
    if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
    if ( (orig_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (orig_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (orig_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_x = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_y = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_z = (double *)malloc( (sum_tri+1) * sizeof(double))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_nx = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_ny = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
    if ( (sort_nz = (int *)malloc( (sum_tri+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed \n\n" ); 
  
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_x[i]=tri[i].cg[0];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_x[i]=rsort[i].r;
      sort_nx[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_y[i]=tri[i].cg[1];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_y[i]=rsort[i].r;
      sort_ny[i]=rsort[i].i;
    }
    for(i=0; i<sum_tri; i++)
    {
      rsort[i].r=orig_z[i]=tri[i].cg[2];
      rsort[i].i=i;
    }
    qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
    for(i=0; i<sum_tri; i++)
    {
      sort_z[i]=rsort[i].r;
      sort_nz[i]=rsort[i].i;
    }

    /* go over all elements and calculate the split-point */
    for (ll=0; ll<set[setdep].anz_e; ll++)
    {
      e=set[setdep].elem[ll];
      //debug_pre_split printf("check elem[%d]:%d type:%d\n", ll, e, e_enqire[e].type);
      if(ecounter>set[setdep].anz_e/10)
      {
        printf("%d from %d elems processed\n", ll+1, set[setdep].anz_e);
        ecounter=0;
      }
      else ecounter++;

      if((e_enqire[e].type==3)||(e_enqire[e].type==6))
      {
        c=j=l=m=k=0;
        /* split the 6 edges and remember the new node per edge-nodes */
        for(i=0; i<6; i++)
        {
          en1=nodseq_te4[j++];
          en2=nodseq_te4[j++];
          n1=e_enqire[e].nod[en1];
          n2=e_enqire[e].nod[en2];
          pl[0][0]=node[n1].nx;
          pl[0][1]=node[n1].ny;
          pl[0][2]=node[n1].nz;
          pl[1][0]=node[n2].nx;
          pl[1][1]=node[n2].ny;
          pl[1][2]=node[n2].nz;
    
          /* The embedded triangles of the surfaces are used as a target */
          /* determine the position where the vector vproj between two line-dots meets the triangles */
          /* and determine if the position is inside the triangle */
          vproj[0]=pl[1][0]-pl[0][0];
          vproj[1]=pl[1][1]-pl[0][1];
          vproj[2]=pl[1][2]-pl[0][2];
          lp0p1=v_betrag( vproj );
          /*
          printf("n1: %lf %lf %lf\n",pl[0][0],pl[0][1],pl[0][2]);
          printf("n2: %lf %lf %lf\n",pl[1][0],pl[1][1],pl[1][2]);
          printf("v: %lf %lf %lf\n",vproj[0],vproj[1],vproj[2]);
          printf("pt1: %lf %lf %lf\n",pt1[0],pt1[1],pt1[2] );
          printf("pt2: %lf %lf %lf\n",pt2[0],pt2[1],pt2[2] );
          printf("pt3: %lf %lf %lf\n",pt3[0],pt3[1],pt3[2] );
          */

          /* determine the center of the edge */
          vn_av[0]=(pl[1][0]+pl[0][0])*.5;
	  vn_av[1]=(pl[1][1]+pl[0][1])*.5;
	  vn_av[2]=(pl[1][2]+pl[0][2])*.5;
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pl[0][0],pl[0][1],pl[0][2],
              sum_tri, &near_node[0], n_closest_nodes/3);
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, vn_av[0],vn_av[1],vn_av[2],
              sum_tri, &near_node[n_closest_nodes/3], n_closest_nodes/3);
          near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, pl[1][0],pl[1][1],pl[1][2],
              sum_tri, &near_node[n_closest_nodes/3*2], n_closest_nodes-n_closest_nodes/3*2);

          /* go over the closest splitting-tri */
          for( nn=0; nn<n_closest_nodes; nn++)
          {
            n=near_node[nn];
	    /*
	    printf("pnt %d %f %f %f\n",n1, pl[0][0], pl[0][1], pl[0][2] );
	    printf("pnt ! %f %f %f\n", vn_av[0], vn_av[1], vn_av[2] );
	    printf("pnt %d %f %f %f\n",n2, pl[1][0], pl[1][1], pl[1][2] );
	    printf("line ! %d %d\n", n1,n2);
	    printf("pnt p%d %f %f %f\n", nn+1, tri[n].cg[0], tri[n].cg[1] , tri[n].cg[2]);
	    if(nn==n_closest_nodes-1) exit(0);
*/
  
            dist=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[0], vproj, 1.01, &orient);
            dist2=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[1], vproj, 1.01, &orient);
    
            /* check if nodes are positioned in (or close to) the splitting plane, these are special cases */
              //debug_pre_split printf(" n1:%d dist:%lf ori:%d \n", n1, dist, orient);
    	      //debug_pre_split printf(" n2:%d dist:%lf ori:%d \n", n2, dist2, orient);
	    
            if ((abs(dist)<=ltol)||(abs(dist2)<=ltol))
	    {
              if(param[0]=='e') { seta(set_eorig, "e", e); goto next_elem; }
              /* was this collapsed node evaluated? */
       	      /* which side is closer to n1 n2? */
              if (abs(dist)<ltol) ns= n1; 
              else ns= n2;
	      for(ii=0; ii<c; ii++) if(ncollapsed[ii]== ns) break;
              if(ii< c) continue;

              ncollapsed[c]=ns;

              //debug_pre_split printf(" node in splitting plane:%d sum_collapsed:%d\n",ncollapsed[c], c+1);

              /* remember the normal vector of the splitting triangle */
              v_result( tri[n].p1,tri[n].p2, v12);
              v_result( tri[n].p1,tri[n].p3, v13);
              v_prod(v12,v13,vn);
              v_norm(vn,&vn_tri[m][0]);

              /* The split-point is just a copy of one end-point of that line */

              /* check if the ns exists already */
              ns=-1;
              for(ii=0; ii<n1ns[n1].sum; ii++) if(n1ns[n1].n2[ii]==n1) { ns=n1ns[n1].nm[ii]; nc=n1ns[n1].nc[ii]; }
              for(ii=0; ii<n1ns[n2].sum; ii++) if(n1ns[n2].n2[ii]==n2) { ns=n1ns[n2].nm[ii]; nc=n1ns[n2].nc[ii]; }
	  		  	
              if(ns==-1)
              {
                /* move n1 or n2 to the splitting plane */
                /* determine normal direction to plane (before the loop starts, common for all elems (TBD)) */
                distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[0], vn, 10.0, &orient);
                if (abs(distn)<ltol)
		{
                  node[n1].nx=pl[0][0]+vn_tri[m][0]*distn;
                  node[n1].ny=pl[0][1]+vn_tri[m][1]*distn;
                  node[n1].nz=pl[0][2]+vn_tri[m][2]*distn;
		}
                distn=v_distA(  tri[n].p1, tri[n].p2, tri[n].p3, pl[1], vn, 10.0, &orient);
                if (abs(distn)<ltol)
	        {
                  node[n2].nx=pl[1][0]+vn_tri[m][0]*distn;
                  node[n2].ny=pl[1][1]+vn_tri[m][1]*distn;
                  node[n2].nz=pl[1][2]+vn_tri[m][2]*distn;
      	        }

		/* which side is closer to n1 n2? */
                if (abs(dist)<ltol) ns= n1; 
                else ns= n2;

                /* copy */
                nc=nod( anz, &node, 1, anz->nmax+1, node[ns].nx, node[ns].ny, node[ns].nz, 0);

		//debug_pre_split printf(" nc:%d distn:%lf ori:%d n:%f %f %f\n", node[nc].nr, distn, orient, vn[0], vn[1], vn[2]);
                if(e_enqire[e].type==6)
	        {
                  if ( (n1nm = (N1nm *)realloc(n1nm, (anz->nmax+1) * sizeof(N1nm))) == NULL )
                  { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].sum=0;
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].n2=n1nm[ii].nm=n1nm[ii].nc=NULL;
		}
                if ( (n1ns[ns].n2 = (int *)realloc( n1ns[ns].n2, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[ns].nm = (int *)realloc( n1ns[ns].nm, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[ns].nc = (int *)realloc( n1ns[ns].nc, (n1ns[ns].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                //if(ns==n1) n1ns[ns].n2[n1ns[ns].sum]=n2; else n1ns[ns].n2[n1ns[ns].sum]=n1;
                n1ns[ns].n2[n1ns[ns].sum]=ns;
                n1ns[ns].nm[n1ns[ns].sum]=node[ns].indx;
                n1ns[ns].nc[n1ns[ns].sum]=nc;
                n1ns[ns].sum++;
                nnew[m]=node[ns].indx;
	      }
              else nnew[m]=ns;
              ncpy[m]=nc;

              /* assign the corner-nodes */
              nref[m][0]=edge[k++]=n1;
              nref[m][1]=edge[k++]=n2;

              //debug_pre_split printf("m:%d c:%d n1:%d n2:%d node:%d copy:%d at  %lf %lf %lf\n", m+1, c+1, n1,n2, node[nnew[m]].nr, node[ncpy[m]].nr,  node[ns].nx, node[ns].ny, node[ns].nz );
              m++;
              c++;
	      goto nextEdge;
	    }

            /* create a node only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
            else if (( dist != -1.e10 ) && ( dist2 != -1.e10 ) && (dist2*dist < 0.))
            {
              if(param[0]=='e') { seta(set_eorig, "e", e); goto next_elem; }

              /* remember the normal vector of the splitting triangle (to be replaced by "orient") */
              v_result( tri[n].p1,tri[n].p2, v12);
              v_result( tri[n].p1,tri[n].p3, v13);
              v_prod(v12,v13,vn);
              v_norm(vn,&vn_tri[m][0]);

              /* check if the ns exists already */
              ns=-1;
              for(ii=0; ii<n1ns[n1].sum; ii++) if(n1ns[n1].n2[ii]==n2) { ns=n1ns[n1].nm[ii]; nc=n1ns[n1].nc[ii]; }
              for(ii=0; ii<n1ns[n2].sum; ii++) if(n1ns[n2].n2[ii]==n1) { ns=n1ns[n2].nm[ii]; nc=n1ns[n2].nc[ii]; }
	  	
              /* create a split-point and a copy if not existent */
              if(ns==-1)
	      {
                v_norm( vproj, pc );
                vs[0]=pc[0]*(dist);
                vs[1]=pc[1]*(dist);
                vs[2]=pc[2]*(dist);
                psplt[0][0]=pl[0][0]+vs[0];
                psplt[0][1]=pl[0][1]+vs[1];
                psplt[0][2]=pl[0][2]+vs[2];
    
    	        ns=nod( anz, &node, 1, anz->nmax+1,  psplt[0][0], psplt[0][1], psplt[0][2], 0);
    	        nc=nod( anz, &node, 1, anz->nmax+1,  psplt[0][0], psplt[0][1], psplt[0][2], 0);

                if(e_enqire[e].type==6)
	        {
                  if ( (n1nm = (N1nm *)realloc(n1nm, (anz->nmax+1) * sizeof(N1nm))) == NULL )
                  { printf("\n\n ERROR in mids: malloc\n\n") ; exit(-1); }    
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].sum=0;
                  for (ii=anz->nmax-2; ii<=anz->nmax; ii++) n1nm[ii].n2=n1nm[ii].nm=n1nm[ii].nc=NULL;
		}
                if ( (n1ns[n1].n2 = (int *)realloc( n1ns[n1].n2, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[n1].nm = (int *)realloc( n1ns[n1].nm, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1ns[n1].nc = (int *)realloc( n1ns[n1].nc, (n1ns[n1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1ns[n1].n2[n1ns[n1].sum]=n2;
                n1ns[n1].nm[n1ns[n1].sum]=ns;
                n1ns[n1].nc[n1ns[n1].sum]=nc;
                n1ns[n1].sum++;
	      }
              nnew[m]=ns;
              ncpy[m]=nc;

              /* assign the corner-nodes */
              nref[m][0]=edge[k++]=n1;
              nref[m][1]=edge[k++]=n2;
    
              //debug_pre_split printf("m:%d n1:%d n2:%d split node:%d copy:%d   %lf %lf %lf\n", m+1, n1,n2, node[nnew[m]].nr,  node[ncpy[m]].nr, psplt[0][0], psplt[0][1], psplt[0][2] );
              m++;
	      goto nextEdge;
            }
          }

          { nunspl[l][0]=n1; nunspl[l++][1]=n2; }
          //debug_pre_split printf("l:%d unsplit nodes:%d %d\n",l, n1,n2 );
 nextEdge:;
        }

        if((e_enqire[e].type==6)&&(m>=2))
        {
          /* remember the corner-nodes of the midside nodes of the original element */
          j=0;
          for(i=0; i<6; i++)
          {
            n1=e_enqire[e].nod[nodseq_te10[j++]];
            nm=node[e_enqire[e].nod[nodseq_te10[j++]]].indx;
            n2=e_enqire[e].nod[nodseq_te10[j++]];
	    //debug_pre_split printf("j:%d n1:%d n2:%d nm:%d\n", j, n1,n2, nm);

            if ( (n1nm[n1].n2 = (int *)realloc( n1nm[n1].n2, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nm = (int *)realloc( n1nm[n1].nm, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            if ( (n1nm[n1].nc = (int *)realloc( n1nm[n1].nc, (n1nm[n1].sum+1) * sizeof(int))) == NULL )
            { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
            n1nm[n1].n2[n1nm[n1].sum]=n2;
            n1nm[n1].nm[n1nm[n1].sum]=nm;
            n1nm[n1].nc[n1nm[n1].sum]=-1;
            n1nm[n1].sum++;
	  }
        }

        /* *******  create new elems ********** */

        if((m==3)&&(c==2)) /* (cut along edge through tet => 2new tets) */
	{
          //debug_pre_split printf("e:%d cut along edge through tet => 2new tets\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* copy element, modify later */
          if(e_enqire[e].type==3)
	  {
            elem_define( anz->emax+1, 3, e_enqire[e].nod, 1, 0 );
	  }
          else
	  {
            elem_define( anz->emax+1, 6, e_enqire[e].nod, 1, 0 );
	  }

          /* search the tetra-top-nodes */
          for(i=0; i<4; i++)
	  {
            nbuf[i]=e_enqire[e].nod[i];
            for(j=0; j<m; j++) if(e_enqire[e].nod[i]==node[nnew[j]].nr) nbuf[i]=-1;
            //debug_pre_split for(j=0; j<m; j++) printf("en:%d nnew %d nbuf %d\n", e_enqire[e].nod[i], node[nnew[j]].nr, nbuf[i]);
	  }
          j=0;
          for(i=0; i<4; i++)
	  {
            if(nbuf[i]!=-1) nbuf[j++]=nbuf[i];
	  }
          //debug_pre_split printf("j:%d top-nodes: %d %d\n", j, nbuf[0], nbuf[1]);
          if(j>2)
          {
            printf("found 3. top-node:%d\n", nbuf[1]);
            for(i=0; i<4; i++) printf("en:%d %d\n", e_enqire[e].nod[i], nbuf[i]);
            exit(0);
          }

          /* prepare the elements */
          /* 1st element gets copied nodes ncpy, oriented in the same way as the plane */
          for(i=0; i<3; i++)
	  {
            seta(set_ntop, "n",node[ncpy[i]].nr);
            seta(set_nbot, "n",node[nnew[i]].nr);
	  }
          seta(set_etop, "e", e);
          seta(set_ebot, "e", anz->emax);

          /* check the orientation of nnew */
          v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[1]].nr].nx, v12);
          v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[2]].nr].nx, v13);
          v_result( &node[node[nnew[0]].nr].nx, &node[edge[i]].nx, v15);
          v_prod(v12,v13,vn);
          v_norm(vn,vn);
          if (v_sprod(vn_av,vn) > 0.)
	  {
            ss=1;
            j=3;
            for(i=0; i<3; i++)
	    {
              e_enqire[e].nod[i]=node[ncpy[i]].nr;
              e_enqire[anz->emax].nod[i]=node[nnew[--j]].nr;
	    }
	  }
	  else
	  {
            ss=0;
            j=3;
            for(i=0; i<3; i++)
	    {
              e_enqire[e].nod[i]=node[ncpy[--j]].nr;
              e_enqire[anz->emax].nod[i]=node[nnew[i]].nr;
	    }
	  }

          /* 1st element has copied nodes and nbuf on the positive side of the plane */
          v_result( &node[node[nnew[0]].nr].nx,&node[nbuf[0]].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
              e_enqire[e].nod[3]=nbuf[0];
              e_enqire[anz->emax].nod[3]=nbuf[1];
	  }
          else
          {
              e_enqire[e].nod[3]=nbuf[1];
              e_enqire[anz->emax].nod[3]=nbuf[0];
	  }

          /* change the midnodes */
          if(e_enqire[e].type==6)
          {
            /* first for the copied elem anz->emax (this has the orig-nodes)  and only the basis (nnew,ncpy) */

            /* gen mitside-nodes */
            for (n=0; n<3; n++)
            {
              nn1= e_enqire[anz->emax].nod[nodseq_te4[n*2]];
              nn2= e_enqire[anz->emax].nod[nodseq_te4[n*2+1]];
              //debug_pre_split printf("e:%d n12:%d %d \n", e, nn1,nn2 );
        
              /* check if the nm and nc exist already */
              nm=nc=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; }
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i]; }
        
              if(nc==-1)
              {
                if(nm==-1)
                {
                  /* generate new node */
                  nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                  node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                  node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                  node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                  if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                  { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                  if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                  { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                  n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                  n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                  n1nm[nn1].sum++;
                }        
                /* generate new copy node */
                nc=nod( anz, &node, 1, anz->nmax+1, node[node[nm].nr].nx,node[node[nm].nr].ny,node[node[nm].nr].nz, 0 );
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].nc[n1nm[nn1].sum]=nc;
              }        
              e_enqire[anz->emax].nod[nodseq_te10[n*3+1]]=node[nm].nr;
              seta(set_nbot, "n",node[nm].nr);
              //debug_pre_split printf("e:%d p:%d n12:%d %d nm:%d\n", anz->emax, nodseq_te10[n*3+1], nn1,nn2, node[nm].nr );
              switch(n)
		{ case 0: jj=5; break;  case 1: jj=4; break; case 2: jj=6; break; default: printf("ERROR: n:%d not possible\n",n); exit(0); }
              e_enqire[e].nod[jj]=node[nc].nr;
              seta(set_ntop, "n",node[nc].nr);
              //debug_pre_split printf("e:%d p:%d n12:%d %d nc:%d\n", e, jj, nn1,nn2, node[nc].nr );
            }

            /* edges running from base to top */
            ec=e;
            for (ee=0; ee<2; ee++)
	    {
	     if(ee==1) ec=anz->emax;
             for (n=0; n<3; n++)
             {
            /* collapsed nodes require a first check with the original nodes */
              nn1= node[nnew[n]].nr;
              nn2= e_enqire[ec].nod[nodseq_te4[(n+3)*2+1]];
              if(ee==0)
            {
                if((nn1!=ncollapsed[0])&&(nn1!=ncollapsed[1])) nn1= node[ncpy[n]].nr;
            }
        
              /* check if the nm exist already (nc never used here because its not splittet running to TOP) */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }        
              if(ee==ss) e_enqire[ec].nod[9-n]=node[nm].nr;
              else e_enqire[ec].nod[n+7]=node[nm].nr;
              //debug_pre_split printf("e:%d p:%d n12:%d %d nm:%d\n", ec, n+7, nn1,nn2, node[nm].nr );
             }        
	    }
	  }
	}
        else if((m==1)&&(c==1)) /* 1 node copied (cut at top) */
	{
          //debug_pre_split printf("e:%d cut at top\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          if(node[nnew[0]].nr==nref[0][0]) n=nref[0][1]; else n=nref[0][0];
          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(i=0; i<4; i++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[i]==node[nnew[ii]].nr)
		{
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i],node[ncpy[ii]].nr );
	          e_enqire[e].nod[i]=node[ncpy[ii]].nr;
                }
            seta(set_ntop, "n",node[ncpy[0]].nr);
            seta(set_etop, "e", e);
          }
          else
          {
            seta(set_nbot, "n",node[nnew[0]].nr);
            seta(set_ebot, "e", e);
          }
	}
        else if((m==3)&&(c==3)) /* 3 nodes copied (cut at basis) */
	{
          //debug_pre_split printf("e:%d cut at basis\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          if(node[nnew[0]].nr==nref[0][0]) n=nref[0][1]; else n=nref[0][0];
          //printf("topn:%d bottomn;%d\n", n, node[nnew[0]].nr);
          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(i=0; i<4; i++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[i]==node[nnew[ii]].nr)
		{
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i],node[ncpy[ii]].nr );
	          e_enqire[e].nod[i]=node[ncpy[ii]].nr;
                }

            /* copy also the midside nodes. */
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                /* check if the nc exists already */
                nc=nm=s=-1;
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; n=nn1; s=i; break; }
  	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i];  n=nn2; s=i; break; } } }
		//debug_pre_split printf("nc:%d nm:%d\n", nc, nm);
    
                if(nm==-1)
                { printf("\n\n ERROR in midnode not found\n\n") ; exit(-1); }    
                if(nc==-1)
                {
                  /* generate new mid-node */
                  nc=nod( anz, &node, 1, anz->nmax+1, node[node[nm].nr].nx, node[node[nm].nr].ny, node[node[nm].nr].nz, 0 );
                  n1nm[n].nc[s]=nc;
                }

		//debug_pre_split printf("nc:%d nod:%d\n", nc,node[nc].nr);
                /* redefine the tet */
                for(i=4; i<10; i++) if(e_enqire[e].nod[i]==node[nm].nr)
                {
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i], node[nc].nr );
                  e_enqire[e].nod[i]=node[nc].nr;
                  break;
                }
                seta(set_ntop, "n",node[nm].nr);
              }
	    }
            for(ii=0; ii<3; ii++) seta(set_ntop, "n",node[ncpy[ii]].nr);
            seta(set_etop, "e",e);
          }
	  else
	  {
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                nm=s=-1;
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; s=i; break; }
    	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; break; } } }
  	        //debug_pre_split printf("nm:%d\n", nm);
                seta(set_nbot, "n",node[nm].nr);
              }
	    }
            for(ii=0; ii<3; ii++) seta(set_nbot, "n",node[nnew[ii]].nr);
            seta(set_ebot, "e", e);
          }

	}
        else if((m==2)&&(c==2)) /* 2 node copied (cut along an edge) */
	{
          //debug_pre_split printf("e:%d cut along an edge\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* sort the elements according to their orientation regarding the splitting triangle */
          /* the element on the fronside of the plane gets the copied nodes */
          n=-1; 
          for(i=0; i<4; i++) if((node[nnew[0]].nr!=e_enqire[e].nod[i])&&(node[nnew[1]].nr!=e_enqire[e].nod[i])) { n=e_enqire[e].nod[i]; break; }
          if(n==-1)
	  { printf("ERROR\n"); exit(0); }
          //debug_pre_split printf("topn:%d bottomn;%d\n", n, node[nnew[0]].nr);


          v_result( &node[node[nnew[0]].nr].nx,&node[n].nx, v12);
          if (v_sprod(vn_av,v12) > 0.)
          {
            /* redefine the tet */
            for(jj=0; jj<4; jj++)
              for(ii=0; ii<m; ii++)
                if(e_enqire[e].nod[jj]==node[nnew[ii]].nr)
	        {
                  //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[jj], node[ncpy[ii]].nr );
                  e_enqire[e].nod[jj]=node[ncpy[ii]].nr;
		}

            /* copy also the midside nodes. */
	    if(e_enqire[e].type==6)
            {
              /* copy the midside nodes */
              jj=0;
              for(ii=0; ii<m; ii++)
  	      {
                nn1=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
                nn2=node[nnew[jj++]].nr;
                if(jj==m) jj=0;
		//debug_pre_split printf("nn1:%d nn2:%d\n", nn1,nn2);

                /* check if the nc exists already */
                nc=nm=s=-1;
		//debug_pre_split printf("nn1sum:%d nn2sum:%d\n", n1nm[nn1].sum,n1nm[nn2].sum);
                for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; nc=n1nm[nn1].nc[i]; n=nn1; s=i; break; }
  	        if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; nc=n1nm[nn2].nc[i];  n=nn2; s=i; break; } } }
		//debug_pre_split printf("nc:%d nm:%d\n", nc, nm);
    
                if(nm==-1)
                { printf("\n\n ERROR in midnode not found\n\n") ; exit(-1); }    
                if(nc==-1)
                {
                  /* generate new mid-node */
                  nc=nod( anz, &node, 1, anz->nmax+1, node[node[nm].nr].nx, node[node[nm].nr].ny, node[node[nm].nr].nz, 0 );
                  n1nm[n].nc[s]=nc;
                }

		//debug_pre_split printf("nc:%d nod:%d\n", nc,node[nc].nr);
                /* redefine the tet */
                for(i=4; i<10; i++)
                  if(e_enqire[e].nod[i]==node[nm].nr)
                  {
                    //debug_pre_split printf("e:%d n:%d to %d\n", e, e_enqire[e].nod[i], node[nc].nr );
                    e_enqire[e].nod[i]=node[nc].nr;
                    break;
                  }
              }
              seta(set_ntop, "n",node[nc].nr);
	    }
            seta(set_ntop, "n",node[ncpy[0]].nr);
            seta(set_ntop, "n",node[ncpy[1]].nr);
            seta(set_etop, "e", e);
          }
	  else
	  {
            nn1=node[nnew[0]].nr;
            nn2=node[nnew[1]].nr;
	    if(e_enqire[e].type==6)
            {
              nm=s=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) { nm=n1nm[nn1].nm[i]; s=i; break; }
    	      if(s<0) { for(i=0; i<n1nm[nn2].sum; i++) { if(n1nm[nn2].n2[i]==nn1) { nm=n1nm[nn2].nm[i]; break; } } }
              seta(set_nbot, "n",node[nm].nr);
            }
            seta(set_nbot, "n",nn1);
            seta(set_nbot, "n",nn2);
            seta(set_ebot, "e", e);
          }

	}
        else if(m==4)  /* 4 edges splitted: 2 pentas */
        {
          //debug_pre_split printf("4 edges splitted: 2 pentas\n",e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* the two unsplitted lines must be unconnected */
          if((nunspl[0][0]==nunspl[1][0])||(nunspl[0][0]==nunspl[1][1])) goto next_elem; 
          if((nunspl[0][1]==nunspl[1][0])||(nunspl[0][1]==nunspl[1][1])) goto next_elem; 

          /* sort the elements according to their orientation regarding the splitting triangle */
          v_result( &node[nunspl[0][0]].nx,&node[nunspl[1][0]].nx, v15);
          v_result( &node[nunspl[0][1]].nx,&node[nunspl[1][1]].nx, v13);
          v_norm(v15,v15);
          v_norm(v13,v13);
          v_add(v15,v13,v12);
          if (v_sprod(vn_av,v12) < 0.) { n1=1; n2=0; } else { n1=0; n2=1; }
 
          /* 1. penta */
          d1=1; d2=4;
          nel[0]=nunspl[n1][0];
          nel[3]=nunspl[n1][1];
          for(i=0; i<4; i++)
          {
            for(j=0; j<2; j++)
            {
	      //printf("ref:%d unspl:%d %d\n", nref[i][j], nunspl[n1][0],nunspl[n1][1]);
	      if(nref[i][j]==nunspl[n1][0])  nel[d1++]=node[nnew[i]].nr;
	      if(nref[i][j]==nunspl[n1][1])  nel[d2++]=node[nnew[i]].nr;
	    }
	  }

          /* check the orientation */
          v_result( &node[nel[0]].nx, &node[nel[1]].nx, v12);
          v_result( &node[nel[0]].nx, &node[nel[2]].nx, v13);
          v_result( &node[nel[0]].nx, &node[nunspl[n1][1]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n > bgrenz) { ibuf=nel[1]; nel[1]=nel[2]; nel[2]=ibuf ; }

          /* check the orientation */
          v_result( &node[nel[3]].nx, &node[nel[4]].nx, v12);
          v_result( &node[nel[3]].nx, &node[nel[5]].nx, v13);
          v_result( &node[nel[3]].nx, &node[nunspl[n1][0]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n < bgrenz) { ibuf=nel[4]; nel[4]=nel[5]; nel[5]=ibuf ; }

          if(e_enqire[e].type==6)
	  {
            /* gen mitside-nodes */
            for (n=0; n<6; n++) nbuf[n]=nel[n];

            for (n=0; n<9; n++)
            {
              nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
              nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];

              /* check if the nm exists already */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
  
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }

              nel[nodseq_pe15[n*3+1]]=node[nm].nr;
            }

            /* add midface node */
            for (n=0; n<2; n++)  
            {
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[0+n]].nx+node[nel[1+n]].nx    +
                node[nel[4+n]].nx+node[nel[3+n]].nx )  + 1./2.*(
                node[nel[6+n]].nx+node[nel[10+n]].nx   +
                node[nel[12+n]].nx+node[nel[ 9+n]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[0+n]].ny+node[nel[1+n]].ny    +
                node[nel[4+n]].ny+node[nel[3+n]].ny )  + 1./2.*(
                node[nel[6+n]].ny+node[nel[10+n]].ny   +
                node[nel[12+n]].ny+node[nel[ 9+n]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[0+n]].nz+node[nel[1+n]].nz    +
                node[nel[4+n]].nz+node[nel[3+n]].nz )  + 1./2.*(
                node[nel[6+n]].nz+node[nel[10+n]].nz   +
                node[nel[12+n]].nz+node[nel[ 9+n]].nz) ;
        
              nel[n+15]=anz->nmax;
            }
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[2]].nx+node[nel[0]].nx    +
                node[nel[3]].nx+node[nel[5]].nx )  + 1./2.*(
                node[nel[ 8]].nx+node[nel[ 9]].nx   +
                node[nel[14]].nx+node[nel[11]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[2]].ny+node[nel[0]].ny    +
                node[nel[3]].ny+node[nel[5]].ny )  + 1./2.*(
                node[nel[ 8]].ny+node[nel[ 9]].ny   +
                node[nel[14]].ny+node[nel[11]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[2]].nz+node[nel[0]].nz    +
                node[nel[3]].nz+node[nel[5]].nz )  + 1./2.*(
                node[nel[ 8]].nz+node[nel[ 9]].nz   +
                node[nel[14]].nz+node[nel[11]].nz) ;
        
              nel[17]=anz->nmax;
        
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[0]].nx+node[nel[2]].nx    +
                node[nel[1]].nx+node[nel[0]].nx )  + 1./2.*(
                node[nel[ 8]].nx+node[nel[ 7]].nx   +
                node[nel[ 6]].nx+node[nel[ 0]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[0]].ny+node[nel[2]].ny    +
                node[nel[1]].ny+node[nel[0]].ny )  + 1./2.*(
                node[nel[ 8]].ny+node[nel[ 7]].ny   +
                node[nel[ 6]].ny+node[nel[ 0]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[0]].nz+node[nel[2]].nz    +
                node[nel[1]].nz+node[nel[0]].nz )  + 1./2.*(
                node[nel[ 8]].nz+node[nel[ 7]].nz   +
                node[nel[ 6]].nz+node[nel[ 0]].nz) ;
        
              nel[18]=anz->nmax;
        
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[3]].nx+node[nel[4]].nx    +
                node[nel[5]].nx+node[nel[3]].nx )  + 1./2.*(
                node[nel[12]].nx+node[nel[13]].nx   +
                node[nel[14]].nx+node[nel[ 3]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[3]].ny+node[nel[4]].ny    +
                node[nel[5]].ny+node[nel[3]].ny )  + 1./2.*(
                node[nel[12]].ny+node[nel[13]].ny   +
                node[nel[14]].ny+node[nel[ 3]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[3]].nz+node[nel[4]].nz    +
                node[nel[5]].nz+node[nel[3]].nz )  + 1./2.*(
                node[nel[12]].nz+node[nel[13]].nz   +
                node[nel[14]].nz+node[nel[ 3]].nz) ;
        
              nel[19]=anz->nmax;
            
            elem_define( e, 5, nel, 1, 0 );
            seta(set_nbot, "n",nel[1]);
            seta(set_nbot, "n",nel[2]);
            seta(set_nbot, "n",nel[4]);
            seta(set_nbot, "n",nel[5]);
            seta(set_nbot, "n",nel[7]);
            seta(set_nbot, "n",nel[10]);
            seta(set_nbot, "n",nel[11]);
            seta(set_nbot, "n",nel[13]);
	  }
          else
          {
            elem_define( e, 2, nel, 1, 0 );
            seta(set_nbot, "n",nel[1]);
            seta(set_nbot, "n",nel[2]);
            seta(set_nbot, "n",nel[4]);
            seta(set_nbot, "n",nel[5]);
          }
          seta(set_ebot, "e",e);

          /* 2. penta, redefine tet */
          d1=1; d2=4;
          for(i=0; i<4; i++)
          {
            nel[0]=nunspl[n2][0];
            nel[3]=nunspl[n2][1];
            for(j=0; j<2; j++)
            {
	      //printf("ref:%d unspl:%d %d\n", nref[i][j], nunspl[n2][0],nunspl[n2][1]);
	      if(nref[i][j]==nunspl[n2][0])  nel[d1++]=node[ncpy[i]].nr;
	      if(nref[i][j]==nunspl[n2][1])  nel[d2++]=node[ncpy[i]].nr;
	    }
	  }
	  //for(i=0; i<6; i++) printf("%d ", nel[i]); printf("\n");

          /* check the orientation */
          v_result( &node[nel[0]].nx, &node[nel[1]].nx, v12);
          v_result( &node[nel[0]].nx, &node[nel[2]].nx, v13);
          v_result( &node[nel[0]].nx, &node[nunspl[n2][1]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n > bgrenz) { ibuf=nel[1]; nel[1]=nel[2]; nel[2]=ibuf ; }

          /* check the orientation */
          v_result( &node[nel[3]].nx, &node[nel[4]].nx, v12);
          v_result( &node[nel[3]].nx, &node[nel[5]].nx, v13);
          v_result( &node[nel[3]].nx, &node[nunspl[n2][0]].nx, v15);
          v_prod(v12,v13,vn);
          v_result(v15,vn,v15n);
          bvn=v_betrag(vn);
          bv15=v_betrag(v15);
          bgrenz=sqrt(bvn*bvn+bv15*bv15);
          bv15n=v_betrag(v15n);

          //printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
          if (bv15n < bgrenz) { ibuf=nel[4]; nel[4]=nel[5]; nel[5]=ibuf ; }

          if(e_enqire[e].type==5)
	  {
            /* gen mitside-nodes */
            for (n=0; n<6; n++) nbuf[n]=nel[n];

            for (n=0; n<9; n++)
            {
              nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
              nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];
 
              /* check if the nm exists already */
              nm=-1;
              for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
              for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
  
              if(nm==-1)
              {
                /* generate new node */
                nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                n1nm[nn1].sum++;
              }

              nel[nodseq_pe15[n*3+1]]=node[nm].nr;
            }

            /* add midface node */
            for (n=0; n<2; n++)  
            {
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[0+n]].nx+node[nel[1+n]].nx    +
                node[nel[4+n]].nx+node[nel[3+n]].nx )  + 1./2.*(
                node[nel[6+n]].nx+node[nel[10+n]].nx   +
                node[nel[12+n]].nx+node[nel[ 9+n]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[0+n]].ny+node[nel[1+n]].ny    +
                node[nel[4+n]].ny+node[nel[3+n]].ny )  + 1./2.*(
                node[nel[6+n]].ny+node[nel[10+n]].ny   +
                node[nel[12+n]].ny+node[nel[ 9+n]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[0+n]].nz+node[nel[1+n]].nz    +
                node[nel[4+n]].nz+node[nel[3+n]].nz )  + 1./2.*(
                node[nel[6+n]].nz+node[nel[10+n]].nz   +
                node[nel[12+n]].nz+node[nel[ 9+n]].nz) ;
        
              nel[n+15]=anz->nmax;
            }
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[2]].nx+node[nel[0]].nx    +
                node[nel[3]].nx+node[nel[5]].nx )  + 1./2.*(
                node[nel[ 8]].nx+node[nel[ 9]].nx   +
                node[nel[14]].nx+node[nel[11]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[2]].ny+node[nel[0]].ny    +
                node[nel[3]].ny+node[nel[5]].ny )  + 1./2.*(
                node[nel[ 8]].ny+node[nel[ 9]].ny   +
                node[nel[14]].ny+node[nel[11]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[2]].nz+node[nel[0]].nz    +
                node[nel[3]].nz+node[nel[5]].nz )  + 1./2.*(
                node[nel[ 8]].nz+node[nel[ 9]].nz   +
                node[nel[14]].nz+node[nel[11]].nz) ;
        
              nel[17]=anz->nmax;
        
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[0]].nx+node[nel[2]].nx    +
                node[nel[1]].nx+node[nel[0]].nx )  + 1./2.*(
                node[nel[ 8]].nx+node[nel[ 7]].nx   +
                node[nel[ 6]].nx+node[nel[ 0]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[0]].ny+node[nel[2]].ny    +
                node[nel[1]].ny+node[nel[0]].ny )  + 1./2.*(
                node[nel[ 8]].ny+node[nel[ 7]].ny   +
                node[nel[ 6]].ny+node[nel[ 0]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[0]].nz+node[nel[2]].nz    +
                node[nel[1]].nz+node[nel[0]].nz )  + 1./2.*(
                node[nel[ 8]].nz+node[nel[ 7]].nz   +
                node[nel[ 6]].nz+node[nel[ 0]].nz) ;
        
              nel[18]=anz->nmax;
        
              nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
              node[anz->nmax].pflag=1;
        
              node[anz->nmax].nx = -1./4.* (
                node[nel[3]].nx+node[nel[4]].nx    +
                node[nel[5]].nx+node[nel[3]].nx )  + 1./2.*(
                node[nel[12]].nx+node[nel[13]].nx   +
                node[nel[14]].nx+node[nel[ 3]].nx) ;
        
              node[anz->nmax].ny = -1./4.* (
                node[nel[3]].ny+node[nel[4]].ny    +
                node[nel[5]].ny+node[nel[3]].ny )  + 1./2.*(
                node[nel[12]].ny+node[nel[13]].ny   +
                node[nel[14]].ny+node[nel[ 3]].ny) ;
        
              node[anz->nmax].nz = -1./4.* (
                node[nel[3]].nz+node[nel[4]].nz    +
                node[nel[5]].nz+node[nel[3]].nz )  + 1./2.*(
                node[nel[12]].nz+node[nel[13]].nz   +
                node[nel[14]].nz+node[nel[ 3]].nz) ;
        
              nel[19]=anz->nmax;
            
            elem_define( anz->emax+1, 5, nel, 1, 0 );
            seta(set_ntop, "n",nel[1]);
            seta(set_ntop, "n",nel[2]);
            seta(set_ntop, "n",nel[4]);
            seta(set_ntop, "n",nel[5]);
            seta(set_ntop, "n",nel[7]);
            seta(set_ntop, "n",nel[10]);
            seta(set_ntop, "n",nel[11]);
            seta(set_ntop, "n",nel[13]);
	  }
          else
          {
            elem_define( anz->emax+1, 2, nel, 1, 0 );
            seta(set_ntop, "n",nel[1]);
            seta(set_ntop, "n",nel[2]);
            seta(set_ntop, "n",nel[4]);
            seta(set_ntop, "n",nel[5]);
          }
          seta(set_etop, "e", anz->emax);
	}
	else if(m==3)
        {
	  //debug_pre_split printf("c:%d e:%d cut at top through tet => 1tet 1pe\n",c, e);

          /* calc average normal of the splitting triangles */
          for (j=0; j<3; j++) vn_av[j]=0.;
          for (i=0; i<m; i++)
	  {
            for (j=0; j<3; j++) vn_av[j]+=vn_tri[i][j];
	  }

          /* search common node of splitted edges, this will be the tip-node of the resulting tet */
          /* exclude all nodes on collapsed edges */
          /* and later replace also nref with nodes from the edge running from ncollapsed to the tip (at n) */
          for(n=0; n<m; n++)
          {
            if((nref[n][0]==ncollapsed[0])||(nref[n][1]==ncollapsed[0])) { edge[n*2]=edge[n*2+1]=0; break; }
	  } 
          qsort( edge, 6, sizeof(int), (void *)compareInt );
          l=0;
          for(i=0;i<5; i++)
          {
            if(edge[i]==edge[i+1]) l++; else l=0;
	    //debug_pre_split printf("node[%d]:%d l:%d\n",i,edge[i],l);
            if((l==1)&&(edge[i]))
            {
              /* tip of tet was found */
              if(c) { nref[n][0]=ncollapsed[0]; nref[n][1]=edge[i]; }
              /* sort the elements according to their orientation regarding the splitting triangle */
              //v_result( &node[node[nnew[0]].nr].nx,&node[edge[i]].nx, v12);
              //if (v_sprod(vn_av,v12) < 0.)
              v_result( &node[node[nnew[0]].nr].nx,&node[edge[i]].nx, v15);
              v_result( &node[node[nnew[1]].nr].nx,&node[edge[i]].nx, v13);
              v_result( &node[node[nnew[2]].nr].nx,&node[edge[i]].nx, v12);
              v_norm(v15,v15);
              v_norm(v13,v13);
              v_norm(v12,v12);
              v_add(v15,v13,vn);
              v_add(vn,v12,v15);

              if (v_sprod(vn_av,v15) < 0.)
              {
                n1=0;
              }
              else
              {
                n1=1;
	      }
	      

              /* check the orientation */
              v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[1]].nr].nx, v12);
              v_result( &node[node[nnew[0]].nr].nx, &node[node[nnew[2]].nr].nx, v13);
              v_result( &node[node[nnew[0]].nr].nx, &node[edge[i]].nx, v15);
              v_prod(v12,v13,vn);
              v_result(v15,vn,v15n);
              bvn=v_betrag(vn);
              bv15=v_betrag(v15);
              bgrenz=sqrt(bvn*bvn+bv15*bv15);
              bv15n=v_betrag(v15n);

	      /*	      
          printf("ntip:%d n:%d\n", edge[i],node[nnew[0]].nr);
	  printf("tri: %f %f %f %f\n", vn_tri[0][0], vn_tri[0][1], vn_tri[0][2], v_betrag(vn_av));
	  printf("v12: %f %f %f %f\n", v12[0], v12[1], v12[2], v_betrag(v12));
	  printf("v15: %f %f %f %f\n", v15[0], v15[1], v15[2], bv15);
              printf ("%f %f vn x=%e y=%e z=%e\n",bgrenz, bv15n,vn[0],vn[1],vn[2]); 
              printf("common node:%d\n",edge[i+1]);
	      */

              if (bv15n < bgrenz)
              {
                /* nodes for the penta elem */
                for(j=0; j<3; j++) 
                {
		  if(n1) nel[j+3]=node[nnew[j]].nr;
		  else   nel[j+3]=node[ncpy[j]].nr;
                  if( edge[i]==nref[j][0]) nel[j]=nref[j][1]; else nel[j]=nref[j][0];

                  /* for collapsed penta */
                  if(!n1) if(c) if(nel[j]==ncollapsed[0]) ibuf=nel[j]=node[ncpy[j]].nr; 
                }
                /* redefine the tet */
                m=3; j=0;
                if(n1) while(j<3) e_enqire[e].nod[m--]=node[ncpy[j++]].nr;
                else   while(j<3) e_enqire[e].nod[m--]=node[nnew[j++]].nr;

                /* for collapsed penta on the tet-side */
                if(n1) if(c) for(j=0; j<3; j++) if(node[nnew[j]].nr==ncollapsed[0]) { ibuf=node[ncpy[j]].nr; break; }
              }         
              else
              {
                /* nodes for the penta elem */
                for(j=0; j<3; j++) 
                {
                  if(n1) nel[j]=node[nnew[j]].nr;
                  else   nel[j]=node[ncpy[j]].nr;
                  if( edge[i]==nref[j][0]) nel[j+3]=nref[j][1]; else nel[j+3]=nref[j][0];

                  /* for collapsed penta */
                  if(!n1) if(c) if(nel[j+3]==ncollapsed[0])  ibuf=nel[j+3]=node[ncpy[j]].nr;
                }
                /* redefine the tet */
                m=1; j=0;
                if(n1)  while(j<3) e_enqire[e].nod[m++]=node[ncpy[j++]].nr;
                else    while(j<3) e_enqire[e].nod[m++]=node[nnew[j++]].nr;

                /* for collapsed penta on the tet-side */
                if(n1) if(c) for(j=0; j<3; j++) if(node[nnew[j]].nr==ncollapsed[0]) { ibuf=node[ncpy[j]].nr; break; }
              }
              e_enqire[e].nod[0]=edge[i];

              if(e_enqire[e].type==6)
	      {
                /* gen mitside-nodes */
                for (n=0; n<6; n++)
                {
                  nn1= e_enqire[e].nod[nodseq_te4[n*2]];
                  nn2= e_enqire[e].nod[nodseq_te4[n*2+1]];

                  /* for collapsed penta on the tet-side */
                  if((n1)&&(c)&&((n<4)&&(n!=1)))
                  {
                    if(nn1==ibuf) nn1=ncollapsed[0];
                    if(nn2==ibuf) nn2=ncollapsed[0];
                  }
                  //debug_pre_split printf("b n1:%d c:%d tet:%d n12 %d %d col %d ibuf %d\n", n1, c, e, nn1, nn2, ncollapsed[0], ibuf);
        
                  /* check if the nm exists already */
                  nm=-1;
                  for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
                  for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
                  if(nm==-1)
                  {
                    /* generate new node */
                    nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                    node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                    node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                    node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                    if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                    { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                    n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                    n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                    n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                    n1nm[nn1].sum++;
                  }        
                  e_enqire[e].nod[nodseq_te10[n*3+1]]=node[nm].nr;
                  if((n>3)||(n==1))
                  {
                    if(!n1) seta(set_nbot, "n",node[nm].nr);
                    else   seta(set_ntop, "n",node[nm].nr);
                  }
                }

                /* penta */

                for (n=0; n<6; n++) nbuf[n]=nel[n];
                for (n=0; n<9; n++)
                {
                  nel[nodseq_pe15[n*3]]=  nn1= nbuf[nodseq_pe6[n*2]];
                  nel[nodseq_pe15[n*3+2]]=   nn2= nbuf[nodseq_pe6[n*2+1]];

                  /* for collapsed penta */
                  if(nn1==nn2)
		  {
                    nel[nodseq_pe15[n*3+1]]=nn1;
		  }
                  else
		  {
                    /* for collapsed penta */
                  //debug_pre_split printf("n1:%d c:%d ibuf:%d col:%d n12 %d %d\n", n1,c, ibuf, ncollapsed[0], nn1, nn2);
                    if(!n1) if(c) { if(nn1==ibuf) nn1=ncollapsed[0]; if(nn2==ibuf) nn2=ncollapsed[0]; }

                    /* check if the nm exists already */
                    nm=-1;
                    for(i=0; i<n1nm[nn1].sum; i++) if(n1nm[nn1].n2[i]==nn2) nm=n1nm[nn1].nm[i];
                    for(i=0; i<n1nm[nn2].sum; i++) if(n1nm[nn2].n2[i]==nn1) nm=n1nm[nn2].nm[i];
        
                    if(nm==-1)
                    {
                      /* generate new node */
                      nm=nod( anz, &node, 1, anz->nmax+1, 0., 0., 0., 0 );
                      node[node[nm].nr].nx=(node[nn1].nx+node[nn2].nx)*.5;
                      node[node[nm].nr].ny=(node[nn1].ny+node[nn2].ny)*.5;
                      node[node[nm].nr].nz=(node[nn1].nz+node[nn2].nz)*.5;
                      if ( (n1nm[nn1].n2 = (int *)realloc( n1nm[nn1].n2, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      if ( (n1nm[nn1].nm = (int *)realloc( n1nm[nn1].nm, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      if ( (n1nm[nn1].nc = (int *)realloc( n1nm[nn1].nc, (n1nm[nn1].sum+1) * sizeof(int))) == NULL )
                      { printf("\n\n ERROR in mids: realloc\n\n") ; exit(-1); }    
                      n1nm[nn1].n2[n1nm[nn1].sum]=nn2;
                      n1nm[nn1].nm[n1nm[nn1].sum]=nm;
                      n1nm[nn1].nc[n1nm[nn1].sum]=-1;
                      n1nm[nn1].sum++;
                    }
                    nel[nodseq_pe15[n*3+1]]=node[nm].nr;
                  
                    if (bv15n < bgrenz)
                    {
                    if((n>2)&&(n<6))
                    {
                        if(n1) seta(set_nbot, "n",node[nm].nr);
                        else   seta(set_ntop, "n",node[nm].nr);
                    }
                  }
                    else
                    {
                    if(n<3)
                    {
                        if(n1) seta(set_nbot, "n",node[nm].nr);
                        else   seta(set_ntop, "n",node[nm].nr);
                    }
                  }
                  
                  }
                }
    
                /* add midface node */
                for (n=0; n<2; n++)  
                {
                  nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
                  node[anz->nmax].pflag=1;
            
                  node[anz->nmax].nx = -1./4.* (
                    node[nel[0+n]].nx+node[nel[1+n]].nx    +
                    node[nel[4+n]].nx+node[nel[3+n]].nx )  + 1./2.*(
                    node[nel[6+n]].nx+node[nel[10+n]].nx   +
                    node[nel[12+n]].nx+node[nel[ 9+n]].nx) ;
            
                  node[anz->nmax].ny = -1./4.* (
                    node[nel[0+n]].ny+node[nel[1+n]].ny    +
                    node[nel[4+n]].ny+node[nel[3+n]].ny )  + 1./2.*(
                    node[nel[6+n]].ny+node[nel[10+n]].ny   +
                    node[nel[12+n]].ny+node[nel[ 9+n]].ny) ;
            
                  node[anz->nmax].nz = -1./4.* (
                    node[nel[0+n]].nz+node[nel[1+n]].nz    +
                    node[nel[4+n]].nz+node[nel[3+n]].nz )  + 1./2.*(
                    node[nel[6+n]].nz+node[nel[10+n]].nz   +
                    node[nel[12+n]].nz+node[nel[ 9+n]].nz) ;
            
                  nel[n+15]=anz->nmax;
                }
                  nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
                  node[anz->nmax].pflag=1;
            
                  node[anz->nmax].nx = -1./4.* (
                    node[nel[2]].nx+node[nel[0]].nx    +
                    node[nel[3]].nx+node[nel[5]].nx )  + 1./2.*(
                    node[nel[ 8]].nx+node[nel[ 9]].nx   +
                    node[nel[14]].nx+node[nel[11]].nx) ;
            
                  node[anz->nmax].ny = -1./4.* (
                    node[nel[2]].ny+node[nel[0]].ny    +
                    node[nel[3]].ny+node[nel[5]].ny )  + 1./2.*(
                    node[nel[ 8]].ny+node[nel[ 9]].ny   +
                    node[nel[14]].ny+node[nel[11]].ny) ;
            
                  node[anz->nmax].nz = -1./4.* (
                    node[nel[2]].nz+node[nel[0]].nz    +
                    node[nel[3]].nz+node[nel[5]].nz )  + 1./2.*(
                    node[nel[ 8]].nz+node[nel[ 9]].nz   +
                    node[nel[14]].nz+node[nel[11]].nz) ;
            
                  nel[17]=anz->nmax;
            
                  nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
                  node[anz->nmax].pflag=1;
            
                  node[anz->nmax].nx = -1./4.* (
                    node[nel[0]].nx+node[nel[2]].nx    +
                    node[nel[1]].nx+node[nel[0]].nx )  + 1./2.*(
                    node[nel[ 8]].nx+node[nel[ 7]].nx   +
                    node[nel[ 6]].nx+node[nel[ 0]].nx) ;
            
                  node[anz->nmax].ny = -1./4.* (
                    node[nel[0]].ny+node[nel[2]].ny    +
                    node[nel[1]].ny+node[nel[0]].ny )  + 1./2.*(
                    node[nel[ 8]].ny+node[nel[ 7]].ny   +
                    node[nel[ 6]].ny+node[nel[ 0]].ny) ;
            
                  node[anz->nmax].nz = -1./4.* (
                    node[nel[0]].nz+node[nel[2]].nz    +
                    node[nel[1]].nz+node[nel[0]].nz )  + 1./2.*(
                    node[nel[ 8]].nz+node[nel[ 7]].nz   +
                    node[nel[ 6]].nz+node[nel[ 0]].nz) ;
            
                  nel[18]=anz->nmax;
            
                  nod( anz, &node, 0, anz->nmax+1, 0., 0., 0., 0 ); 
                  node[anz->nmax].pflag=1;
            
                  node[anz->nmax].nx = -1./4.* (
                    node[nel[3]].nx+node[nel[4]].nx    +
                    node[nel[5]].nx+node[nel[3]].nx )  + 1./2.*(
                    node[nel[12]].nx+node[nel[13]].nx   +
                    node[nel[14]].nx+node[nel[ 3]].nx) ;
            
                  node[anz->nmax].ny = -1./4.* (
                    node[nel[3]].ny+node[nel[4]].ny    +
                    node[nel[5]].ny+node[nel[3]].ny )  + 1./2.*(
                    node[nel[12]].ny+node[nel[13]].ny   +
                    node[nel[14]].ny+node[nel[ 3]].ny) ;
            
                  node[anz->nmax].nz = -1./4.* (
                    node[nel[3]].nz+node[nel[4]].nz    +
                    node[nel[5]].nz+node[nel[3]].nz )  + 1./2.*(
                    node[nel[12]].nz+node[nel[13]].nz   +
                    node[nel[14]].nz+node[nel[ 3]].nz) ;
            
                  nel[19]=anz->nmax;

                elem_define( anz->emax+1, 5, nel, 1, 0 );

	      }
	      else
              {
                elem_define( anz->emax+1, 2, nel, 1, 0 );
              }

              for(j=0; j<3; j++) { seta(set_ntop, "n",node[ncpy[j]].nr); seta(set_nbot, "n",node[nnew[j]].nr); }
              if (n1==0)
              {
                seta(set_ebot, "e", e);
                seta(set_etop, "e",anz->emax);
              }
              else
              {
                seta(set_etop, "e", e);
                seta(set_ebot, "e",anz->emax);
	      }

              break;
	    }
          }

        }
	else if(m>0)
        {
	  printf("c:%d e:%d m:%d unknow case\n",c, e, m);
	  goto next_elem;
	}
      }
      continue;
    next_elem:;
     seta(set_efail, "e",e);
    }
    if(param[0]=='e') goto ready;

    /* new number of original nodes */
    anz->orignmax = anz->nmax;
    anz->orign = anz->n;

    completeSet( "all", "up" );
    completeSet( "all", "do" );
    
    /* new midnodes */
    adjustDrawNodes(1);

    /* merge top nodes */
    sprintf(buffer,"n %s",set[set_ntop].name);
    pre_merge(buffer);

    makeSurfaces();
    getElemNormalen( e_enqire, node, anz->e );
    realloc_colNr();

    if(printFlag) printf(" updateDispLists\n");
    updateDispLists(); 

    completeSet( set[set_ntop].name, "do" );
    completeSet( set[set_nbot].name, "do" );
    separateMeshes( "all");

  ready:;
    printf (" ready\n");
  }


  /* 1) split all lines */

  /* go over all lines */
  if(set[setNr].anz_l)
  {
    /* create a dummy set for the new lines */
    setSplit=pre_seta("-splitbuf","i",0);

    /* go over all lines and calculate the split-point */
    for (ll=0; ll<set[setNr].anz_l; ll++)
    {
      l=set[setNr].line[ll];
      if(printFlag) printf("check line:%s\n", line[l].name);

      /* store the line-dots in an array of points */
      /* the original coordinates are used  */
      lps=0.;
      ltot=calcLineLength(l)/scale->w;

      pl[0][0]=line[l].ip[0];
      pl[0][1]=line[l].ip[1];
      pl[0][2]=line[l].ip[2];
      for (p=3; p<line[l].nip; p+=3)
      {
        pl[1][0]=line[l].ip[p]  ;
        pl[1][1]=line[l].ip[p+1];
        pl[1][2]=line[l].ip[p+2];
  
        /* The embedded triangles of the surfaces are used as a target */
        /* determine the position where the vector vproj between two line-dots meets the triangles */
        /* and determine if the position is inside the triangle */
        vproj[0]=pl[1][0]-pl[0][0];
        vproj[1]=pl[1][1]-pl[0][1];
        vproj[2]=pl[1][2]-pl[0][2];
        lp0p1=v_betrag( vproj );
        /*
        printf("pl: %lf %lf %lf\n",pl[0][0],pl[0][1],pl[0][2]);
        printf("v: %lf %lf %lf\n",vproj[0],vproj[1],vproj[2]);
        printf("pt1: %lf %lf %lf\n",pt1[0],pt1[1],pt1[2] );
        printf("pt2: %lf %lf %lf\n",pt2[0],pt2[1],pt2[2] );
        printf("pt3: %lf %lf %lf\n",pt3[0],pt3[1],pt3[2] );
        */

        /* go over all splitting-tri */
        for( n=0; n<sum_tri; n++)
        {
          pt1[0]=tri[n].p1[0];
          pt1[1]=tri[n].p1[1];
          pt1[2]=tri[n].p1[2];
          pt2[0]=tri[n].p2[0];
          pt2[1]=tri[n].p2[1];
          pt2[2]=tri[n].p2[2];
          pt3[0]=tri[n].p3[0];
          pt3[1]=tri[n].p3[1];
          pt3[2]=tri[n].p3[2];
  
          dist=v_distA(  pt1, pt2, pt3, pl[0], vproj, 1., &orient);
          dist2=v_distA(  pt1, pt2, pt3, pl[1], vproj, 1., &orient);

          /* create a point only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
	  /* printf("nip:%d dist2:%lf dist:%lf p:%d \n",line[l].nip, dist2, dist, p); */
          if (( dist != -1.e10 ) && ( dist2 != -1.e10 ) && (dist2*dist < 0.))
          {
            /* split-point */
            v_norm( vproj, pc );
            vs[0]=pc[0]*(dist);
            vs[1]=pc[1]*(dist);
            vs[2]=pc[2]*(dist);
            psplt[0][0]=pl[0][0]+vs[0];
            psplt[0][1]=pl[0][1]+vs[1];
            psplt[0][2]=pl[0][2]+vs[2];
            lp0p1=v_betrag( vs );
            lps+=lp0p1;

            /* split line only if its end-points are more than ltol away */
            mode=0;
            dx=psplt[0][0]-(point[line[l].p1].px);
            dy=psplt[0][1]-(point[line[l].p1].py);
            dz=psplt[0][2]-(point[line[l].p1].pz);
            if(sqrt(dx*dx+dy*dy+dz*dz)<ltol) mode=1;
	    /* printf("d1:%lf gtol:%lf\n", sqrt(dx*dx+dy*dy+dz*dz),ltol); */
            if(!mode)
	    {
              dx=psplt[0][0]-(point[line[l].p2].px);
              dy=psplt[0][1]-(point[line[l].p2].py);
              dz=psplt[0][2]-(point[line[l].p2].pz);
              if(sqrt(dx*dx+dy*dy+dz*dz)<ltol) mode=1;
	      /* printf("d2:%lf gtol:%lf\n", sqrt(dx*dx+dy*dy+dz*dz),ltol); */
	    }
            if(!mode)
	    {
              getNewName( name, "p" );
              if(printFlag) printf(" create split-point:%s %lf %lf %lf\n", name, psplt[0][0], psplt[0][1], psplt[0][2] );
              ps_nr  = pnt( name, psplt[0][0], psplt[0][1], psplt[0][2], 0 );
  
              /* create two new lines l_nr */
              splitLine(l, l_nr, ps_nr, lps/ltot);
              seta(setSplit, "l", l_nr[0]);
              seta(setSplit, "l", l_nr[1]);

              /* remember the basic line for later deletion */
              if ( (lbuffer = (int *)realloc((int *)lbuffer, (sum_l+1)*sizeof(int))) == NULL ) printf("\n\n ERROR: malloc failure\n\n" );
              lbuffer[sum_l]=l;
              sum_l++;
  
              /* untersuche alle lcmbs ob linbuf ein Mitglied ist */
              for (i=0; i<anzGeo->c; i++)
              {
                for (j=0; j<lcmb[i].nl; j++)
                {
                  if( l == lcmb[i].l[j] )
                  {
                    printf (" realloc lcmb:%s and replace line:%s with %s and %s \n",
                      lcmb[i].name, line[l].name,line[l_nr[0]].name, line[l_nr[1]].name );
                    if ((lcmb[i].o = (char *)realloc( (char *)lcmb[i].o, (lcmb[i].nl+1)*sizeof(char)) ) == NULL )
                    { printf("\n\n ERROR: realloc failure in qspl, lcmb.o:%s not changed\n\n",lcmb[i].name ); return; }
                    if ((lcmb[i].l = (int *)realloc( (int *)lcmb[i].l, (lcmb[i].nl+1)*sizeof(int)) ) == NULL )
                    { printf("\n\n ERROR: realloc failure in qspl, lcmb.l:%s not changed\n\n", lcmb[i].name); return; }
  
                    /* umspeichern der linien beginnend bei der letzten bis einschlieslich j */
                    for (k=lcmb[i].nl; k>j; k--)
            	    {
                      lcmb[i].o[k]=lcmb[i].o[k-1];
                      lcmb[i].l[k]=lcmb[i].l[k-1];
                    }
  
                    /* Auffuellen der j, j+1 pos. mit l1, l2 mit der Orientierung der gesplitteten linie */
                    lcmb[i].o[j]=lcmb[i].o[j+1];
                    lcmb[i].l[j]=l_nr[0];
                    lcmb[i].o[j+1]=lcmb[i].o[j];
                    lcmb[i].l[j+1]=l_nr[1];
                    lcmb[i].nl++;
                  }
                }
              }

              /* untersuche alle surfs ob linbuf ein Mitglied ist und ersetze sie durch eine lcmb */
              /* kontrolliere ob nicht schon eine geeignete lcmb existiert */
              for (i=0; i<anzGeo->s; i++)
              {
                for (j=0; j<surf[i].nl; j++)
                {
                  if(( l == surf[i].l[j] )&&( surf[i].typ[j]=='l' ))
                  {
                    /* do we already have a suitable lcmb? */
                    for (k=0; k<anzGeo->c; k++ )
            	    {
                      if (lcmb[k].nl==2)   /* same amount of lines */
            	      {
            	        /*
                        printf ("check lcmb:%s \n", lcmb[k].name);
            	        */
                        if (((lcmb[k].l[0]==l_nr[0])||(lcmb[k].l[0]==l_nr[1])) && ((lcmb[k].l[1]==l_nr[0])||(lcmb[k].l[1]==l_nr[1])))
            	        {
            	          /*
                          printf ("equal:%s\n",lcmb[k].name);
            	          */
                          break;
            	        }
            	      }
                    }
                    if (k>=anzGeo->c)  /* no lcmb was found, so create one */
                    {
                      /* create lcmb */
                      getNewName( name, "c" );
                      lcmb_i( name, (int)0, (int)2, "++", l_nr );
                      k=getLcmbNr( name );
                    }
                    if(printFlag) printf ("realloc surf:%s and replace line:%s with lcmb:%s made of %s and %s \n",
                      surf[i].name, line[l].name, name, line[l_nr[0]].name, line[l_nr[1]].name );
                    if (k>-1) { surf[i].l[j]=k; surf[i].o[j]='+'; surf[i].typ[j]='c'; }
                    else { errMsg("lcmb not known, surface could not be changed \n"); return; }
                  }
                }
              }
            }
            goto nextLine;  // check no more tri's, take the next line
	  }
        }
        lps+=lp0p1;
        pl[0][0]=pl[1][0];
        pl[0][1]=pl[1][1];
        pl[0][2]=pl[1][2];
      }
    nextLine:;
    }
    /* delete all splitted lines */
    delLine( sum_l, lbuffer );
    free(lbuffer);
    lbuffer=NULL;
    sum_l=0;
    /* add the new lines to the split set */
    s=getSetNr(setname);
    for(ll=0; ll<set[setSplit].anz_l; ll++) seta(s,"l",set[setSplit].line[ll]);
    delSet(set[setSplit].name);
  }


  /* 2) split all surfaces */

  /* go over all surfs which should be splitted (opposite to the lines-procedere) */
  /* and create the split-points */
  for (ss=0; ss<set[setNr].anz_s; ss++)
  {
    s=set[setNr].surf[ss];
    /* go over all tri */
    n=0;
    while((surf[s].npgn-n))
    {
      /* requires that only triangles are used for the interiour description of the surface */
      /* this is regarded by the adjustFeedBack() routine */
      n+=5; /* jump over the leading bytes */
      ps[0][0]=surf[s].pgn[n++];
      ps[0][1]=surf[s].pgn[n++];
      ps[0][2]=surf[s].pgn[n++];
      ps[1][0]=surf[s].pgn[n++];
      ps[1][1]=surf[s].pgn[n++];
      ps[1][2]=surf[s].pgn[n++];
      ps[2][0]=surf[s].pgn[n++];
      ps[2][1]=surf[s].pgn[n++];
      ps[2][2]=surf[s].pgn[n++];

      /* "cg" of tri-edges */
      cg_edge[0][0]=(ps[0][0]+ps[1][0])*.5;
      cg_edge[0][1]=(ps[0][1]+ps[1][1])*.5;
      cg_edge[0][2]=(ps[0][2]+ps[1][2])*.5;

      cg_edge[1][0]=(ps[1][0]+ps[2][0])*.5;
      cg_edge[1][1]=(ps[1][1]+ps[2][1])*.5;
      cg_edge[1][2]=(ps[1][2]+ps[2][2])*.5;

      cg_edge[2][0]=(ps[2][0]+ps[0][0])*.5;
      cg_edge[2][1]=(ps[2][1]+ps[0][1])*.5;
      cg_edge[2][2]=(ps[2][2]+ps[0][2])*.5;

      /* go over all edges and determine the split-point */
      sum_psplt=0;
      for(e=0; e<3; e++)
      {
        /* find the closest splitting-tri */
        /* distance between line-cg and tri-cg */
        if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
          printf("ERROR: realloc failed: Rsort\n\n" );
        for(i=0; i<sum_tri; i++)
        {
          dx=tri[i].cg[0]-cg_edge[e][0];
          dy=tri[i].cg[1]-cg_edge[e][1];
          dz=tri[i].cg[2]-cg_edge[e][2];
          rsort[i].r=dx*dx+dy*dy+dz*dz;
          rsort[i].i=i;
        }
        qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );

        //for (i=0; i<sum_tri; i++) printf("%d n:%d r:%lf\n", i, rsort[i].i, rsort[i].r); 

        /* check the closest tri for a split-point */
        for(i=0; i<sum_tri; i++)
        {
          /* The embedded triangles of the surfaces are used as a target */
          /* determine the position where the vector vproj between two line-dots meets the triangles */
          /* and determine if the position is inside the triangle */
          if(e==0)      { p2_nr=1; p1_nr=0; }
          else if(e==1) { p2_nr=2; p1_nr=1; }
          else          { p2_nr=0; p1_nr=2; }
          vproj[0]=ps[p2_nr][0]-ps[p1_nr][0];
          vproj[1]=ps[p2_nr][1]-ps[p1_nr][1];
          vproj[2]=ps[p2_nr][2]-ps[p1_nr][2];
          dist=v_distA(  tri[rsort[i].i].p1, tri[rsort[i].i].p2, tri[rsort[i].i].p3, ps[p1_nr], vproj, 1., &orient);
          dist2=v_distA( tri[rsort[i].i].p1, tri[rsort[i].i].p2, tri[rsort[i].i].p3, ps[p2_nr], vproj, 1., &orient);
  
          /* create a point only if the vector goes through the tri and if the sign of dist has changed (scip the first line-vector (p==3))*/
          //printf(" tri:%d dist2:%lf dist:%lf\n",rsort[i].i, dist2, dist);
          if (( dist != -1.e10 ) && ( dist2 != -1.e10 ) && (dist2*dist < 0.))
          {
            /* split-point */
            v_norm( vproj, pc );
            psplt[sum_psplt][0]=ps[p1_nr][0]+pc[0]*(dist);
            psplt[sum_psplt][1]=ps[p1_nr][1]+pc[1]*(dist);
            psplt[sum_psplt][2]=ps[p1_nr][2]+pc[2]*(dist);
            sum_psplt++;
            break;
	  }
    	}
      }

      /* create split-point inside the surface-tri */
      if(sum_psplt==2)
      {
        pc[0]=(psplt[0][0]+psplt[1][0])*.5;
        pc[1]=(psplt[0][1]+psplt[1][1])*.5;
        pc[2]=(psplt[0][2]+psplt[1][2])*.5;
        getNewName( name, "p" );
        if(printFlag) printf(" create surf-point:%s %lf %lf %lf\n", name, pc[0], pc[1], pc[2] );
        ps_nr  = pnt( name, pc[0], pc[1], pc[2], 0 );
      }
    }
  }

  /* split-points are created, create a new line */
  /* tbd */

  /* lines are created, create new surfaces */
  /* tbd */

  free(tri);
  free(rsort);
  delSet(specialset->dep);
  delSet(specialset->ind);
}


/*
  if(compare(action,"PROJ",4)==4) functionFlag=0;
  else functionFlag=1; ("DIST")
*/
void pre_proj( char *string)
{
#define INTPOLMODE  0 /* linear */
#define PROJ_BIAS 1.1
#define PROJ_LOOPS 10

  int  length, i,j=0,n,e,p, setNr, trgtNr, mode, csys[3], surfFlag=0, normFlag=0, functionFlag, counter, orient;
  char setname[MAX_LINE_LENGTH], targetname[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH];
  char axis;
  double pr[3], pc[3], nr[3], pa1[3], pa2[3], pa3[3], ps1[3], ps2[3], vproj[3],v1[3],v2[3],bv1,bv2, offset=0., dist=1.e33, mindist=0, mindistini=0.;
  int  minj=0, anz_n=0;
  double pt1[3], pt2[3], pt3[3], rp, triScale=0.;
  double *nx=NULL;
  double *nrad=NULL;
  int  *nmissed=NULL;

  char pkt1[MAX_LINE_LENGTH], pkt2[MAX_LINE_LENGTH];
  int   pnr1=0, pnr2=0, setdep, setind;
  double x, y, z, dx,dy,dz, l, h, q, l_offs, h_offs, q_offs;
  double p1[3], p2[3], p1p2[3];
  double el[3], eh[3], eq[3];   /* xyz koordinaten der lhq vektoren */
  double ex[3], ey[3], ez[3];   /* lhq koordinaten der xyz vektoren */
  double ph[3], p1ph[3], pp[3], e1[3], lhq[3], angle;
  double pval, qval, divpq, scal_e[2];

  double dist_max=-MAX_FLOAT,dist_min=MAX_FLOAT, sum_dist=0.;
  double dist_maxx=-MAX_FLOAT,dist_minx=MAX_FLOAT, distx;
  double dist_maxy=-MAX_FLOAT,dist_miny=MAX_FLOAT, disty;
  double dist_maxz=-MAX_FLOAT,dist_minz=MAX_FLOAT, distz;
  int ndist=0;

  Rsort *rsort=NULL;
  Tri *tri=NULL;
  int sum_tri=0;

  Points    *pproj=NULL;
  int sum_pproj;
  char *record;
 
  char parameter[20][MAX_LINE_LENGTH];

  /* get the function name */
  i=0; while(string[i]==' ') i++;
  i+= sword( string, action);
  record=string+i;

  /* check if the function 'dist' or 'proj' was selected */
  for(i=0; i<strlen(action); i++) action[i]=toupper(action[i]);
  if(compare(action,"PROJ",4)==4) functionFlag=1;
  else functionFlag=0;

  /* get basic parameters */
  action[0]=0;
  length = sscanf( record,"%s %s %s", setname, targetname, action );
  operateAlias( setname, "se" );
  setNr=getSetNr( setname );
  if (setNr<0)
  {
    errMsg(" ERROR: Set (%s) is undefined\n", setname );
    return;
  }

  delSet(specialset->dep );
  delSet(specialset->ind );


  /* first assume the target is a single shape */
  operateAlias( targetname, "sh" );
  trgtNr=getShapeNr( targetname );
  if (trgtNr>-1)
  {
    if( (setind=pre_seta( specialset->ind, "sh", targetname )) <0 )
    {
      printf("ERROR: shape:%s could not be used\n", shape[trgtNr].name);
      return;
    }
    trgtNr=-1;
  }
  else
  {
    operateAlias( targetname, "se" );
    trgtNr=getSetNr( targetname );
    if (trgtNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", targetname );
      return;
    }

    /* cycle through all entities of targetNr and add them to the special set  */
    /* cyrcle through all bodies and add  */
    if( (setind=pre_seta( specialset->ind, "i", 0 )) <0 ) return;
    for (i=0; i<set[trgtNr].anz_s; i++)
    {
      seta( setind, "s", set[trgtNr].surf[i] );
    }
    for (i=0; i<set[trgtNr].anz_l; i++)
    {
      seta( setind, "l", set[trgtNr].line[i] );
    }
    for (i=0; i<set[trgtNr].anz_f; i++)
    {
      seta( setind, "f", set[trgtNr].face[i] );
    }
    for (i=0; i<set[trgtNr].anz_n; i++)
    {
      seta( setind, "n", set[trgtNr].node[i] );
    }
    for (i=0; i<set[trgtNr].anz_sh; i++)
    {
      seta( setind, "sh", set[trgtNr].shp[i] );
    }
    /* add only shell elements */
    for (i=0; i<set[trgtNr].anz_e; i++)
    {
      if((e_enqire[set[trgtNr].elem[i]].type >6)&&(e_enqire[set[trgtNr].elem[i]].type <11)) seta( setind, "e", set[trgtNr].elem[i] );
    }
    /* second cycle through all entities and add lower ones  to the special set  */
    completeSet( specialset->ind, "do") ;
    completeSet( specialset->ind, "do") ;
  }

  /* stop here if just a measurement between points or nodes should be performed */
  if( (!functionFlag)&&( (set[setNr].anz_p==1)||(set[setNr].anz_n==1)) && (action[0]==0) )
  {
    /* loop over all points or nodes of the target */
    if(set[setNr].anz_p)
      nrad=&point[set[setNr].pnt[0]].px;
    else if(set[setNr].anz_n)
      nrad=&node[set[setNr].node[0]].nx;
    else return;
    if(set[trgtNr].anz_p)
    {
      for(n=0; n<set[trgtNr].anz_p; n++)
      {
        nx=&point[set[trgtNr].pnt[n]].px;
        v_result(nx,nrad,pr);
        v_scal(&scale->w,pr,pr );
        dist=v_betrag(pr);
        distx=pr[0];
        disty=pr[1];
        distz=pr[2];
        if(dist>dist_max) { dist_max=dist; pnr1=n; }
        if(dist<dist_min) { dist_min=dist; pnr2=n; }
        if(distx>dist_maxx) { dist_maxx=distx; }
        if(distx<dist_minx) { dist_minx=distx; }
        if(disty>dist_maxy) { dist_maxy=disty; }
        if(disty<dist_miny) { dist_miny=disty; }
        if(distz>dist_maxz) { dist_maxz=distz; }
        if(distz<dist_minz) { dist_minz=distz; }
      }
      printf("DISTMAX:%f xyz:%f %f %f\n", dist_max, dist_maxx, dist_maxy, dist_maxz );
      sprintf(parameter[0],"%f",dist_max);
      sprintf(parameter[1],"%f",dist_maxx);
      sprintf(parameter[2],"%f",dist_maxy);
      sprintf(parameter[3],"%f",dist_maxz);
      printf("DISTMIN:%f xyz:%f %f %f\n", dist_min, dist_minx, dist_miny, dist_minz );
      sprintf(parameter[4],"%f",dist_min);
      sprintf(parameter[5],"%f",dist_minx);
      sprintf(parameter[6],"%f",dist_miny);
      sprintf(parameter[7],"%f",dist_minz);
    }
    else if(set[trgtNr].anz_n)
    {
      for(n=0; n<set[trgtNr].anz_n; n++)
      {
        nx=&node[set[trgtNr].node[n]].nx;
        v_result(nx,nrad,pr);
        v_scal(&scale->w,pr,pr );
        dist=v_betrag(pr);
        if(dist>dist_max) { dist_max=dist; pnr1=n; }
        if(dist<dist_min) { dist_min=dist; pnr2=n; }
      }
      nx=&node[set[trgtNr].node[pnr1]].nx;
      v_result(nx,nrad,pr);
      v_scal(&scale->w,pr,pr );
      dist=v_betrag(pr);
      printf("DISTMAX:%f xyz:%f %f %f\n", dist,pr[0],pr[1],pr[2] );
      sprintf(parameter[0],"%f",dist);
      sprintf(parameter[1],"%f",pr[0]);
      sprintf(parameter[2],"%f",pr[1]);
      sprintf(parameter[3],"%f",pr[2]);
      nx=&node[set[trgtNr].node[pnr2]].nx;
      v_result(nx,nrad,pr);
      v_scal(&scale->w,pr,pr );
      dist=v_betrag(pr);
      printf("DISTMIN:%f xyz:%f %f %f\n", dist,pr[0],pr[1],pr[2] );
      sprintf(parameter[4],"%f",dist);
      sprintf(parameter[5],"%f",pr[0]);
      sprintf(parameter[6],"%f",pr[1]);
      sprintf(parameter[7],"%f",pr[2]);
    }

    write2stack( 8, parameter);
    return;
  }

  /* cycle through all entities of setNr and add them to the special set  */
  /* cyrcle through all bodys and add  */
  if( (setdep=pre_seta( specialset->dep, "i", 0 )) <0 ) return;
  for (i=0; i<set[setNr].anz_n; i++)
  {
    seta( setdep, "n", set[setNr].node[i] );
  }
  /* add all related elements for a later midside-node-correction */
  completeSet( specialset->dep, "up") ;

  for (i=0; i<set[setNr].anz_b; i++)
  {
    seta( setdep, "b", set[setNr].body[i] );
  }
  /* cyrcle through all surfs and add  */
  for (i=0; i<set[setNr].anz_s; i++)
  {
    seta( setdep, "s", set[setNr].surf[i] );
  }
  /* cyrcle through all lcmbs and add  */
  for (i=0; i<set[setNr].anz_c; i++)
  {
    seta( setdep, "c", set[setNr].lcmb[i] );
  }
  /* cyrcle through all lines and add  */
  for (i=0; i<set[setNr].anz_l; i++)
  {
    seta( setdep, "l", set[setNr].line[i] );
  }
  for (i=0; i<set[setNr].anz_p; i++)
  {
    seta( setdep, "p", set[setNr].pnt[i] );
  }
  /* second cycle through all entities and add lower ones  to the special set  */
  // completeSet( specialset->dep, "do") ;


  if (set[setNr].anz_n==0) set[setdep].anz_n=set[setNr].anz_n;
  setNr=setdep;
  trgtNr=setind;

  /* generate an array of all points or nodes to be projected */
  /* the original coordinates must be used for radial projection to get the correct axis */
  sum_pproj=0;
  for (i=0; i<set[setNr].anz_p; i++)
  {
    p=set[setNr].pnt[i];
    if(point[p].name==(char *)NULL) continue;
    if (p<0)
    {
      errMsg(" ERROR: Point-nr:%d is undefined\n", set[setNr].pnt[i] );
      goto nextpoint;
    }
    if ( (pproj = (Points *)realloc( (Points *)pproj, (sum_pproj+2) * sizeof(Points))) == NULL )
    {
      errMsg("\nERROR: realloc failed in pre_proj() \n\n");
    }
    pproj[sum_pproj].nn=p;
    pproj[sum_pproj].name=point[p].name;
    pproj[sum_pproj].px=point[p].px*scale->w+scale->x;
    pproj[sum_pproj].py=point[p].py*scale->w+scale->y;
    pproj[sum_pproj].pz=point[p].pz*scale->w+scale->z;
    sum_pproj++;
  }
  for (i=0; i<set[setNr].anz_n; i++)
  {
    n=set[setNr].node[i];
    if ( (pproj = (Points *)realloc( (Points *)pproj, (sum_pproj+2) * sizeof(Points))) == NULL )
    {
      errMsg("\nERROR: realloc failed in pre_proj() \n\n");
    }
    pproj[sum_pproj].nn=-n;
    sprintf(name,"%d",n);
    if((pproj[sum_pproj].name= (char *)malloc((strlen(name)+1)*sizeof(char))) == NULL )
    { printf("ERROR: malloc failed\n\n" ); return; }
    strcpy(pproj[sum_pproj].name,name);
    pproj[sum_pproj].px=node[n].nx*scale->w+scale->x;
    pproj[sum_pproj].py=node[n].ny*scale->w+scale->y;
    pproj[sum_pproj].pz=node[n].nz*scale->w+scale->z;
    sum_pproj++;
  }
  if(!sum_pproj) return;

  /* fill all unfilled surfaces, do it in any case because points could have been moved! */
  /* repShape will change *scale and therefore must be executed first */
  repShape(trgtNr);
  for (j=0; j<set[trgtNr].anz_s; j++)
  {
    //orientSet( set[trgtNr].name );
    if(surf[set[trgtNr].surf[j]].sh>-1)
    {
      if(shape[surf[set[trgtNr].surf[j]].sh].type==4)
      {
        repNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]); untrimNurs(shape[surf[set[trgtNr].surf[j]].sh].p[0]);
      }
    }
    repSurf(set[trgtNr].surf[j],1);
  }

  /* generate splitting triangles */
  sum_tri=genSplitTrias(trgtNr, &tri, 1);

  /*
  printf("sum_tri:%d\n",sum_tri);
  for( n=0; n<sum_tri; n++)
  {
    printf(" pnt ! %f %f %f\n", tri[n].p1[0], tri[n].p1[1], tri[n].p1[2]);
    printf(" pnt ! %f %f %f\n", tri[n].p2[0], tri[n].p2[1], tri[n].p2[2]);
    printf(" pnt ! %f %f %f\n", tri[n].p3[0], tri[n].p3[1], tri[n].p3[2]);
  }
  */

  /* project the nodes and points */
  if(getSetNr(specialset->noprj)>-1) delSet(specialset->noprj);
  if( compare( action, "rad", 3) == 3)
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    length = sscanf( record,"%*s %*s %*s %c %lf %lf", &axis, &offset, &mindistini );

    /* target needs lines or triangles */
    if( sum_tri == 0) 
    {
      if( set[trgtNr].anz_l > 0)
      {
        surfFlag=0;
        for (j=0; j<set[trgtNr].anz_l; j++) repLine(set[trgtNr].line[j]);
      }
      else
      {
        errMsg(" ERROR: Set (%s) does not have lines, surfaces or a triangle-mesh\n", targetname );
        return;
      }
    }
    else 
    {
      surfFlag=1;
    }

    for (i=0; i<sum_pproj; i++)
    {
      /* the original coordinates must be used for radial projection to get the correct axis */
      pr[0]=pproj[i].px;
      pr[1]=pproj[i].py;
      pr[2]=pproj[i].pz;

      /* move the point to the line of nodes or to the surface */
      if( surfFlag)
      {
        /* The embedded triangles of the surfaces are used as a target for the projection */
        /* determine the position where the radial-ray through the point meets the triangles */
        /* and determine if the position is inside the triangle */

        if( axis=='x')
        {
          vproj[0]=0.;
          vproj[1]=pr[1];
          vproj[2]=pr[2];
	}
        if( axis=='y')
        {
          vproj[0]=pr[0];
          vproj[1]=0.;
          vproj[2]=pr[2];
	}
        if( axis=='z')
        {
          vproj[0]=pr[0];
          vproj[1]=pr[1];
          vproj[2]=0.;
	}

        /* find the closest tri */
        /* distance between point and tri-cg */
        if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
          printf("ERROR: realloc failed: Rsort\n\n" );
        for(j=0; j<sum_tri; j++)
        {
          dx=tri[j].cg[0]-pr[0];
          dy=tri[j].cg[1]-pr[1];
          dz=tri[j].cg[2]-pr[2];
          rsort[j].r=dx*dx+dy*dy+dz*dz;
          rsort[j].i=j;
        }
        qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );

        counter=0;
        do
        {
          counter++;
  
          /* check the closest tri for a split-point */
          minj=-1;mindist=mindistini; triScale=1.;
          do
          {
  
          for(j=0; j<sum_tri; j++)
          {
            dist=v_distA(  tri[rsort[j].i].p1, tri[rsort[j].i].p2, tri[rsort[j].i].p3, pr, vproj, PROJ_BIAS, &orient);
            if(( dist != -1.e10 )&&(abs(dist)<abs(mindist)))
            {
              if(printFlag)  printf(" found enclosing tri3:%d for point:%s, distance:%lf\n", tri[rsort[j].i].fnr, pproj[i].name, dist); 
              mindist=dist;
              minj=j;
            }
          }
            if( minj!=-1 ) break;
            else triScale+=0.1;
          }while(triScale<=PROJ_BIAS);
  
          if( minj!=-1 )
          {
            j=minj;
            dist=v_distA(  tri[rsort[j].i].p1, tri[rsort[j].i].p2, tri[rsort[j].i].p3, pr, vproj, PROJ_BIAS, &orient);
            /* berechne den Gradeneinheitsvektor en */
            v_norm( vproj, pc );
            pr[0]+=pc[0]*(dist+offset);
            pr[1]+=pc[1]*(dist+offset);
            pr[2]+=pc[2]*(dist+offset);
          }
        }while((dist>gtol/scale->w)&&(counter<PROJ_LOOPS));

        if(minj!=-1 )
        {
          //printf(" found enclosing tri3[%d] for point:%s, distance:%lf triScale:%f break at j=%d  loop:%d\n", rsort[minj].i, pproj[i].name, dist, triScale,minj, counter); 
        }
        else
        {
          if(pproj[i].nn>-1) { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
          else               { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
          goto nextpoint;
        }
      }
      else
      {
        /* Internal points of the target-lines are used as a target for the projection */

        /* count the internal points */
        for (j=0; j<set[trgtNr].anz_l; j++)
        {
          anz_n+=line[set[trgtNr].line[j]].nip;
	}
	anz_n/=3;

        if ( (nx = (double *)malloc( (anz_n+1) * sizeof(double))) == NULL )
        {  errMsg("\n\n ERROR: malloc failed in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; }
        if ( (nrad = (double *)malloc( (anz_n+1) * sizeof(double))) == NULL )
        {  errMsg("\n\n ERROR: malloc failed in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; }

        if( axis=='x') j=0;
        else if( axis=='y') j=1;
        else if( axis=='z') j=2;
        else {  errMsg("\n\n ERROR: axis unknown in pre_proj()\n") ; free_proj(nx, nrad, nmissed); return; };
        if( v_rec2cyl( pr, j, csys, pc) <0) goto nextpoint;

        /* move the point to the target-line */
        e=0;
        for (j=0; j<set[trgtNr].anz_l; j++)
        {
          for (n=0; n<line[set[trgtNr].line[j]].nip; n+=3)
          {
            nr[0]=line[set[trgtNr].line[j]].ip[n]*scale->w+scale->x;
            nr[1]=line[set[trgtNr].line[j]].ip[n+1]*scale->w+scale->y;
            nr[2]=line[set[trgtNr].line[j]].ip[n+2]*scale->w+scale->z;
            nx[e]=nr[csys[0]];
            nrad[e++]=sqrt(nr[csys[1]]*nr[csys[1]]+nr[csys[2]]*nr[csys[2]]);
	    //printf(" target-line %f %f\n", nx[e-1], nrad[e-1]);
          }
        }
        //printf("move the point to the target-line\n");
        mode=INTPOLMODE; 
        pc[0]=intpol2( nx, nrad, e, pc[2], &mode )+offset;
        
	//printf(" csys %d %d %d\n", csys[0], csys[1],csys[2]);
        pr[csys[0]]=pc[2];
        pr[csys[1]]=cos(pc[1])*pc[0];
        pr[csys[2]]=sin(pc[1])*pc[0];
      }
  
      if(functionFlag)
      {
        /* redefine the point */
        pproj[i].px= pr[0];
        pproj[i].py= pr[1];
        pproj[i].pz= pr[2];
      }
      else
      {
        v_result(&pproj[i].px, pr, p1p2);
	//printf(" pp %f %f %f\n", pproj[i].px,pproj[i].py,pproj[i].pz);
	//printf(" pr %f %f %f\n", pr[0], pr[1],pr[2]);

        dist=v_betrag(p1p2);
        // since the distance should be given as distance from target:
        if(v_betrag(pr)>v_betrag(&pproj[i].px)) dist*=-1;
        if(dist>dist_max) dist_max=dist;
        if(dist<dist_min) dist_min=dist;
        sum_dist+=dist;
        ndist++;
        //if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
      }

    nextpoint:;
    }
  }
  else if( compare( action, "rot", 3) == 3)
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    length = sscanf( record, "%*s%*s%*s%s%s%lf%lf", pkt1, pkt2, &offset, &mindistini );

    p1[0] = 0.;
    p1[1] = 0.;
    p1[2] = 0.;
    p2[0] = 0.;
    p2[1] = 0.;
    p2[2] = 0.;

    if(!length) 
    {
      errMsg(" ERROR: not enough parameter specified\n");
      return;
    }
    else if(length==1)
    {
      if(pkt1[0]=='x')   p2[0] = 1.;
      else if(pkt1[0]=='y')   p2[1] = 1.;
      else if(pkt1[0]=='z')   p2[2] = 1.;
      else
      {
        errMsg(" Axis:%s is undefined\n", pkt1 );
        return;
      }
      offset=0.;
    }
    else if(length>=2)
    {
      pnr1=getPntNr( pkt1 );
      pnr2=getPntNr( pkt2 );
      if(pnr2<0)
      {
        if(pkt1[0]=='x')   p2[0] = 1.;
        else if(pkt1[0]=='y')   p2[1] = 1.;
        else if(pkt1[0]=='z')   p2[2] = 1.;
        else
        {
          errMsg(" Axis:%s is undefined\n", pkt1 );
          return;
        }
        offset=atof(pkt2);
      }
      else
      {
        p1[0] = point[pnr1].px*scale->w+scale->x;
        p1[1] = point[pnr1].py*scale->w+scale->y;
        p1[2] = point[pnr1].pz*scale->w+scale->z;
    
        p2[0] = point[pnr2].px*scale->w+scale->x;
        p2[1] = point[pnr2].py*scale->w+scale->y;
        p2[2] = point[pnr2].pz*scale->w+scale->z;
      }
    }
    if(offset!=0)
    {
      printf("\n WARNING option offset not implemented so far. offset:%lf ignored.\n\n",offset);
    }

    /* The embedded triangles are used as a target for the projection */

    /* berechnung der Einheitsvektoren des Verdreh-Koordinatensystems */
    /* Exneu = p1p2/|p1p2| ist der Einheitsvektor in xneu richtung    */
    /* Eyneu = p1p2 X p1ph / |p1p2 X p1ph|     in yneu                */
    /* Ezneu = Exneu X Eyneu                   in zneu                */

    v_result( p1, p2, p1p2 );
    v_norm  ( p1p2, el );
    
    /* erzeuge einen Hilfspunkt der nicht auf der el-achse liegt  */
    ph[1] = p1p2[0];
    ph[2] = p1p2[1];
    ph[0] = p1p2[2];
  
    /* konstuiere damit den 2. einheitsvektor eh  */
    v_result( p1, ph, p1ph );
    v_prod( p1p2, p1ph, ph );
    v_norm (ph, eh);
  
    /* und der dritte: eq  */
    v_prod( p1p2, ph, ex );
    v_norm (ex, eq);

    /* berechnung der lhq-koordinaten der xyz einheitsvektoren durch zykl.vertausch.  */
    ex[0]=el[0];
    ex[1]=eh[0];
    ex[2]=eq[0];

    ey[0]=el[1];
    ey[1]=eh[1];
    ey[2]=eq[1];

    ez[0]=el[2];
    ez[1]=eh[2];
    ez[2]=eq[2];

    /* Berechnung der lhq-koordinaten des Startpunktes der Drehachse (pnr1 ist der 0-Punkt des lhq systems) */
    x=p1[0];
    y=p1[1];
    z=p1[2];
    
    l_offs=ex[0]*x+ey[0]*y+ez[0]*z;
    h_offs=ex[1]*x+ey[1]*y+ez[1]*z;
    q_offs=ex[2]*x+ey[2]*y+ez[2]*z;
    /*
    printf("el:%lf %lf %lf eh:%lf %lf %lf eq:%lf %lf %lf\n", el[0],el[1],el[2], eh[0],eh[1],eh[2], eq[0],eq[1],eq[2]);
    printf("ex:%lf %lf %lf ey:%lf %lf %lf ez:%lf %lf %lf\n", ex[0],ex[1],ex[2], ey[0],ey[1],ey[2], ez[0],ez[1],ez[2]);
    printf("xyz p1:%lf %lf %lf\n", x,y,z);
    printf("lhq p1:%lf %lf %lf\n", l_offs, h_offs, q_offs);
    */

    /* Berechnung der lhq-koordinaten aller zu drehenden punkte */
    for (i=0; i<sum_pproj; i++)
    {
      x=pproj[i].px;
      y=pproj[i].py;
      z=pproj[i].pz;

      pp[0]=l=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;   /* l (drehachse) */
      pp[1]=h=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;   /* h */
      pp[2]=q=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;   /* q */

      /* define a cutting plane pa by 3 points ( pa1,pa2,pa3) in the lhq system */
      pa1[0]=l;
      pa1[1]=0.;
      pa1[2]=0.;
      pa2[0]=l;
      pa2[1]=1.;
      pa2[2]=0.;
      pa3[0]=l;
      pa3[1]=0.;
      pa3[2]=1.;

      /* cycle through all target-surfaces and determine the triangle which fits and move the point */
      /* find the closest tri */
      /* distance between point and tri-cg */
      if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
        printf("ERROR: realloc failed: Rsort\n\n" );
      for(j=0; j<sum_tri; j++)
      {
        dx=tri[j].cg[0]-x;
        dy=tri[j].cg[1]-y;
        dz=tri[j].cg[2]-z;
        rsort[j].r=dx*dx+dy*dy+dz*dz;
        rsort[j].i=j;
      }
      qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );
#if TEST
      for (j=0; j<sum_tri; j++)
        printf("%d n:%d r:%lf\n", j, rsort[j].i, rsort[j].r); 
#endif

      /* check the closest tri for a split-point */
      for(j=0; j<sum_tri; j++)
      {
        x=tri[rsort[j].i].p1[0];
        y=tri[rsort[j].i].p1[1];
        z=tri[rsort[j].i].p1[2];
        pt1[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
        pt1[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
        pt1[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */

        x=tri[rsort[j].i].p2[0];
        y=tri[rsort[j].i].p2[1];
        z=tri[rsort[j].i].p2[2];
        pt2[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
        pt2[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
        pt2[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */

        x=tri[rsort[j].i].p3[0];
        y=tri[rsort[j].i].p3[1];
        z=tri[rsort[j].i].p3[2];
        pt3[0]=( ex[0]*x+ey[0]*y+ez[0]*z ) - l_offs ;    /* l */
        pt3[1]=( ex[1]*x+ey[1]*y+ez[1]*z ) - h_offs ;    /* h */
        pt3[2]=( ex[2]*x+ey[2]*y+ez[2]*z ) - q_offs ;    /* q */

        if( AsplitA( pa1, pa2, pa3, pt1, pt2, pt3, ps1, ps2) >0 )
        {
          /* check if the projected point is on that element */
          /*
	  printf("pnt pp: %lf %lf %lf\n",l,h,q);
	  printf("pnt ps1: %lf %lf %lf\n",ps1[0],ps1[1],ps1[2]);
	  printf("pnt ps2: %lf %lf %lf\n",ps2[0],ps2[1],ps2[2]);
	  printf("pnt pa1: %lf %lf %lf\n",pa1[0],pa1[1],pa1[2]);
	  printf("pnt pa2: %lf %lf %lf\n",pa2[0],pa2[1],pa2[2]);
	  printf("pnt pa3: %lf %lf %lf\n",pa3[0],pa3[1],pa3[2]);
	  printf("pnt pt1: %lf %lf %lf\n",pt1[0],pt1[1],pt1[2] );
	  printf("pnt pt2: %lf %lf %lf\n",pt2[0],pt2[1],pt2[2] );
	  printf("pnt pt3: %lf %lf %lf\n",pt3[0],pt3[1],pt3[2] );
          */

          v_result(ps1,ps2,e1);
          rp=v_norm( e1, e1 );
          /* check if ps1==ps2 then no intersection line could be determined and no projection is possible */
          if(rp>1.e-10)
	  {
            pval=2*(ps1[1]*e1[1]+ps1[2]*e1[2])/(e1[1]*e1[1]+e1[2]*e1[2]);
            qval=ps1[1]*ps1[1]+ps1[2]*ps1[2]-pp[1]*pp[1]-pp[2]*pp[2];
            divpq=pval*pval*0.25-qval;
            if(divpq>=0.)
	    {
              if(printFlag) printf(" found enclosing tri3 for point:%s\n", pproj[i].name);
  
              /* new h and q values  */
              scal_e[0]=-pval*0.5 +sqrt(divpq);
              scal_e[1]=-pval*0.5 -sqrt(divpq);
              //printf("p:%f q:%f scal:%f scal:%f\n", pval, qval, scal_e[0], scal_e[1]); 
              v_scal(&scal_e[0], e1, p1);
              v_add(ps1,p1, p1);
              v_scal(&scal_e[1], e1, p2);
              v_add(ps1,p2, p2);
/*
              printf("pnt e1: %lf %lf %lf\n",e1[0],e1[1],e1[2] );
              printf("pnt pc1: %lf %lf %lf\n",p1[0],p1[1],p1[2] );
              printf("pnt pc2: %lf %lf %lf\n",p2[0],p2[1],p2[2] );
*/  
              /* check which solution gives the smallest correction */
              v_result(pp,p1,v1);
              v_result(pp,p2,v2);
              //printf("v1:%f v2:%f\n",v_betrag(v1),v_betrag(v2)); 
              bv1=v_betrag(v1);
              bv2=v_betrag(v2);

              if(bv1<bv2)
              {
                dist=bv1;
                h=p1[1];
                q=p1[2];
              }
              else
              {
                dist=bv2;
                h=p2[1];
                q=p2[2];
              }
              if(abs(dist)<abs(mindistini)) goto foundTri3rot;
            }
          }
	}
      }
      if(pproj[i].nn>-1) { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
      else               { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
      goto nextpointrot;
     foundTri3rot:;
      
      lhq[0]=l;
      lhq[1]=h;
      lhq[2]=q;
      l+= l_offs;
      h+= h_offs;
      q+= q_offs;
      
      x=el[0]*l+eh[0]*h+eq[0]*q;
      y=el[1]*l+eh[1]*h+eq[1]*q;
      z=el[2]*l+eh[2]*h+eq[2]*q;
  
      if(functionFlag)
      {
        /* redefine the point */
        pproj[i].px=  x;
        pproj[i].py=  y;
        pproj[i].pz=  z;
      }
      else
      {
        pp[0]=lhq[0]=0.;
	angle=v_angle(pp,lhq);

        // generate a signum by try and error
        h=lhq[1]*cos(-angle)-lhq[2]*sin(-angle);
        q=lhq[1]*sin(-angle)+lhq[2]*cos(-angle);
        if((abs(h-pp[1])>1.e-6)||(abs(q-pp[2])>1.e-6)) angle*=-1.;
        dist=angle*180./PI;
        // since the distance should be given as distance from target:
        dist*=-1;
        if(dist>dist_max) dist_max=dist;
        if(dist<dist_min) dist_min=dist;
        sum_dist+=dist;
        ndist++;
	if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
      }
     nextpointrot:;
    }
  }
  else if(( compare( action, "tra", 3) == 3)||( compare( action, "nor", 3) == 3))
  {
    offset=0.;
    mindistini=MAX_FLOAT;
    if( compare( action, "tra", 3) == 3)
      length = sscanf( record,"%*s %*s %*s %lf %lf %lf %lf %lf", &vproj[0], &vproj[1], &vproj[2], &offset, &mindistini );
    else
    {
      length = sscanf( record,"%*s %*s %*s %lf %lf", &offset, &mindistini);
      normFlag=1;
    }

    /* The embedded triangles are the surfaces which are used as a target for the projection */
    for (i=0; i<sum_pproj; i++)
    {
      pr[0]=pproj[i].px;
      pr[1]=pproj[i].py;
      pr[2]=pproj[i].pz;

      /* find the closest tri */
      /* distance between point and tri-cg */
      if ( (rsort = (Rsort *)realloc((Rsort *)rsort, (sum_tri+1) * sizeof(Rsort))) == NULL )
        printf("ERROR: realloc failed: Rsort\n\n" );
      for(j=0; j<sum_tri; j++)
      {
        dx=tri[j].cg[0]-pr[0];
        dy=tri[j].cg[1]-pr[1];
        dz=tri[j].cg[2]-pr[2];
        rsort[j].r=dx*dx+dy*dy+dz*dz;
        rsort[j].i=j;
      }
      qsort( rsort, sum_tri, sizeof(Rsort), (void *)compareRsort );

      counter=0;
      do
      {
        counter++;

        /* check the closest tri for a split-point */
        minj=-1;mindist=mindistini; triScale=1.;
        do
        {
          for(j=0; j<sum_tri; j++)
          {
            if(normFlag)
   	      {
              v_result( tri[rsort[j].i].p1, tri[rsort[j].i].p2, v1);
              v_result( tri[rsort[j].i].p1, tri[rsort[j].i].p3, v2);
              v_prod( v1, v2, vproj );
   	      }
            dist=v_distA(  tri[rsort[j].i].p1, tri[rsort[j].i].p2, tri[rsort[j].i].p3, pr, vproj, triScale, &orient);
            //printf(" tri3:%d r:%f for point:%s dist:%f mindist:%f triScale:%f\n", tri[rsort[j].i].fnr, rsort[j].r, pproj[i].name, dist, mindist, triScale); 
            if(( dist != -1.e10 )&&(abs(dist)<abs(mindist)))
            {
              if(printFlag) printf(" found enclosing tri3:%d for point:%s, distance:%lf %lf\n", tri[rsort[j].i].fnr, pproj[i].name, dist, mindist); 
              mindist=dist;
              minj=j;
            }
          }
          if( minj!=-1 ) break;
          else triScale+=0.1;
        }while(triScale<=PROJ_BIAS);
  
        if( minj!=-1 )
        {
          j=minj;
          if(normFlag)
  	  {
            v_result( tri[rsort[j].i].p1, tri[rsort[j].i].p2, v1);
            v_result( tri[rsort[j].i].p1, tri[rsort[j].i].p3, v2);
            v_prod( v1, v2, vproj );
  	  }
          dist=v_distA(  tri[rsort[j].i].p1, tri[rsort[j].i].p2, tri[rsort[j].i].p3, pr, vproj, triScale, &orient);
        }
      }while((dist>gtol/scale->w)&&(counter<PROJ_LOOPS));

      if(minj!=-1 )
      {
        // printf(" found enclosing tri3[%d] for point:%s, distance:%lf triScale:%f break at j=%d  loop:%d\n", rsort[minj].i, pproj[i].name, dist, triScale,minj, counter); 
        if(functionFlag)
	{
          /* berechne den Gradeneinheitsvektoren */
          v_norm( vproj, pc );
          pr[0]+=pc[0]*(dist+offset);
          pr[1]+=pc[1]*(dist+offset);
          pr[2]+=pc[2]*(dist+offset);

          /* redefine the point */
          pproj[i].px= pr[0];
          pproj[i].py= pr[1];
          pproj[i].pz= pr[2];
	}
        else
	{
          /* berechne den Gradeneinheitsvektoren */
          /*          
          v_norm( vproj, pc );
          pr[0]=pc[0]*(dist+offset);
          pr[1]=pc[1]*(dist+offset);
          pr[2]=pc[2]*(dist+offset);
          dist=v_betrag(pr);
	  */
          // instead keep the sign:
          dist=dist-offset;
          // since the distance should be given as distance from target:
          dist*=-1;
          if(dist>dist_max) dist_max=dist;
          if(dist<dist_min) dist_min=dist;
          sum_dist+=dist;
          ndist++;
	  if(printFlag) printf("dist:%f av:%f ndist:%d max:%f min:%f\n", dist, sum_dist/ndist, ndist, dist_max, dist_min);
	}
      }
      else
      {
        if(pproj[i].nn>-1) { printf("WARNING: no enclosing element for point:%s found\n", pproj[i].name ); pre_seta(specialset->noprj, "p",pproj[i].name ); } 
        else               { printf("WARNING: no enclosing element for node:%s found\n", pproj[i].name );  pre_seta(specialset->noprj, "n",pproj[i].name ); }
      }
    }
  }
  else
  {
    errMsg(" ERROR: transformation:%s not known\n", action);
    return;
  }

  /* map the results to the original entities */
  if(functionFlag) for (i=0; i<sum_pproj; i++)
  {
    if(pproj[i].nn>-1)
    {
      p=pproj[i].nn;
      point[p].px=(pproj[i].px-scale->x)/scale->w;
      point[p].py=(pproj[i].py-scale->y)/scale->w;
      point[p].pz=(pproj[i].pz-scale->z)/scale->w;
    }
    else
    {
      n=-pproj[i].nn;
      node[n].nx=(pproj[i].px-scale->x)/scale->w;
      node[n].ny=(pproj[i].py-scale->y)/scale->w;
      node[n].nz=(pproj[i].pz-scale->z)/scale->w;
    }
  }
  free(pproj);
  free(tri);
  free(rsort);

  if(getSetNr(specialset->noprj)>-1)
  {
    printf("WARNING: check set:%s for failed entities\n", specialset->noprj);
    if(( compare( action, "nor", 3) == 3)&&(set[trgtNr].anz_sh))
    {
      /* at last project normal onto the referenced nurbs-surfaces */

      printf("  trying to project failed entities to nurbs-shapes (successfull proj not removed from %s)\n", specialset->noprj);
      setNr=getSetNr(specialset->noprj);
      for (i=0; i<set[trgtNr].anz_sh; i++)
      {
        if(shape[set[trgtNr].shp[i]].type==4)
          projSetToNurbs( nurbs, shape[set[trgtNr].shp[i]].p[0], set, setNr, point);
      }

      if(getSetNr(specialset->zap)>-1) delSet(specialset->zap);
    }
  }

  if(!functionFlag)
  {
    printf("DIST:%f MAX:%f MIN:%f\n", sum_dist/ndist, dist_max, dist_min);
    free_proj(nx, nrad, nmissed);

    if(valuestackFlag)
    {
      if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+3)*sizeof(char *)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      for(i=0; i<3; i++)
      {
        if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
        { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      }
      sprintf(valuestack[valuestack_ptr++],"%f", dist_min );
      sprintf(valuestack[valuestack_ptr++],"%f", dist_max );
      sprintf(valuestack[valuestack_ptr++],"%f", sum_dist/ndist );
      printf(" in inverse order written to stack\n");
    }
    return;
  }

  /* dependent higher entities need new label-positions, achived with orient */
  /* create a set with the dependent higher entities to be oriented  */

  /* add all points to special set _ORI  */
  if(set[setNr].anz_p>0)
  { 
    j=pre_seta( specialset->ori, "i", 0 );
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      seta( j, "p", p );
    }
    completeSet( specialset->ori, "up");
    orientSet( specialset->ori );
    delSet(specialset->ori);
  }

  /* recalculate the shapes (from command-interpreter "REP") */
  for (i=0; i<anzGeo->l; i++) repLine(i);

  /* fix midside-node positions */
  fixMidsideNodes(set[setNr].name, "");

  /* reposition the internal drawing nodes */
  if(anz->e>0)
  {
    adjustDrawNodes(0);
    //makeSurfaces(); instead:
    getFaceNormalen( face, node, anz );
    getElemNormalen( e_enqire, node, anz->e );
    updateDispLists();
  }
  free_proj(nx, nrad, nmissed);
}


void  generateSetIndexes()
{
  int i, index=1;
  for (i=0; i<anz->sets; i++)
  {
    if( set[i].name != (char *)NULL )
    {
      if (!set[i].type)
      {
        set[i].index=index++;
      }
    }
  }
}


/*------------------------------------------------------------------*/
/* setinhalt schreiben                                              */
/*------------------------------------------------------------------*/

int prnt(char *record)
{
  int   setNr, nr, p, l, nc,nl, args;
  int i,j,k,n;
  int length, bias_fbd, foundSubString;
  char name[MAX_LINE_LENGTH], typ[MAX_LINE_LENGTH], param[MAX_LINE_LENGTH], **dat;
  char parameter[20][MAX_LINE_LENGTH];
  char *str=NULL, *token=NULL, *saveptr=NULL;

  param[0]=0;
  length=sscanf(record,"%s%s%s",typ,name,param);
  if((length==2)&&(compareStrings(name, "*")>0)) length--;

  /* generate the set-indexes */
  generateSetIndexes();

  if ((compare( typ, "usr", 3) == 3) ||(compare( typ, "USR", 3) == 3))
  {
    if (!anz->u) printf(" no user headers defined\n");
    /* list all existing parameters */
    for (i=0; i<anz->u; i++)
    {
        if( anz->uheader[i] != (char *)NULL )
        {
          printf ("%s\n", &anz->uheader[i][6]);
          strcpy(parameter[0], &anz->uheader[i][6]);
          write2stack(1, parameter);
        }
    }
    return(1);
  }
  if ((compare( typ, "par", 3) == 3) ||(compare( typ, "PAR", 3) == 3))
  {
    if (!lcase[cur_lc].npheader) printf(" no parameters for active dataset:%d defined\n", cur_lc+1);
    if (length==1)
    {
      /* list all existing parameters */
      for (i=0; i<lcase[cur_lc].npheader; i++)
      {
        if( lcase[cur_lc].pheader[i] != (char *)NULL )
        {
          printf ("%s\n", &lcase[cur_lc].pheader[i][6]);
          strcpy(parameter[0], &lcase[cur_lc].pheader[i][6]);
          write2stack(1, parameter);
        }
      }
      return(1);
    }
    else
    {
      for (i=0; i<lcase[cur_lc].npheader; i++)
      {
        if( lcase[cur_lc].pheader[i] != (char *)NULL )
        {
          if(compare(&lcase[cur_lc].pheader[i][6],name,strlen(name))==strlen(name))
	  {
            printf ("%s\n", &lcase[cur_lc].pheader[i][6]);
            strcpy(parameter[0], &lcase[cur_lc].pheader[i][6]);
            write2stack(1, parameter);
	  }
        }
      } 
      return(1);
    }
  }

  if ((compare( typ, "amp", 3) == 3) ||(compare( typ, "AMP", 3) == 3))
  {
    if (!anz->amps) printf(" no amplitudes defined\n");
    if (length==1)
    {
      /* list all existing amplitudes */
      for (i=0; i<anz->amps; i++)
      {
        if( amplitude[i].name != (char *)NULL )
        {
          printf ("%s xy-pairs:%d\n", amplitude[i].name, amplitude[i].n);
        }
      }
      return(1);
    }
    else
    {
      setNr=getAmplitudeNr(name,1);
      if (setNr<0)
      {
        /* amp was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->amps; setNr++)
	  {
            for(i=0; i<length; i++)
	    {
              if(strstr(amplitude[setNr].name, dat[i]) !=NULL)
	      {
                if( amplitude[setNr].name != (char *)NULL )
                {
                  printf ("%s xy-pairs:%d\n", amplitude[setNr].name, amplitude[setNr].n);
                }
	      }
	    }
	  }
          return(0);
	}
        printf (" prnt: amplitude:'%s' does not exist\n", name);
        return (-1);
      }
    }
    printf ("i      x:            y:\n");
    for (i=0; i<amplitude[setNr].n; i++)
    {
      printf (" %8d %12e %12e\n", i+1, amplitude[setNr].x[i],amplitude[setNr].y[i]);
    }
  }

  if ((compare( typ, "mat", 3) == 3) ||(compare( typ, "MAT", 3) == 3))
  {
    if (!anz->mats) printf(" no materials defined\n");
    if (length==1)
    {
      /* list all existing amplitudes */
      for (i=0; i<anz->mats; i++)
      {
        if( material[i].name != (char *)NULL )
        {
          printf ("%s elastic:%d expansion:%d conductivity:%d specific heat:%d \n", material[i].name, material[i].nela, material[i].nexp, material[i].ncon, material[i].nsph);
        }
      }
      return(1);
    }
    else
    {
      setNr=getMatNr(name,1);
      if (setNr<0)
      {
        /* amp was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->mats; setNr++)
	  {
            for(i=0; i<length; i++)
	    {
              if(strstr(material[setNr].name, dat[i]) !=NULL)
	      {
                if( material[setNr].name != (char *)NULL )
                {
                  printf ("%s elastic:%d expansion:%d conductivity:%d specific heat:%d \n", material[setNr].name, material[setNr].nela, material[setNr].nexp, material[setNr].ncon, material[setNr].nsph);
                }
	      }
	    }
	  }
          return(0);
	}
        printf (" prnt: material:'%s' does not exist\n", name);
        return(-1);
      }
    }
    printf ("density:%e\n", material[setNr].rho);
    printf ("\ni      elastic:      nue:      temperature:\n");
    for (i=0; i<material[setNr].nela; i++)
    {
      printf (" %8d %12e %12e %12e\n", i+1, material[setNr].ela[i],material[setNr].nue[i],material[setNr].tela[i]);
    }
    printf ("\ni      expansion:      temperature:\n");
    for (i=0; i<material[setNr].nexp; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].exp[i],material[setNr].texp[i]);
    }
    printf ("\ni      conductivity:      temperature:\n");
    for (i=0; i<material[setNr].ncon; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].con[i],material[setNr].tcon[i]);
    }
    printf ("\ni      sp. heat:      temperature:\n");
    for (i=0; i<material[setNr].nsph; i++)
    {
      printf (" %8d %12e %12e\n", i+1, material[setNr].sph[i],material[setNr].tsph[i]);
    }
  }

  if ((compare( typ, "st", 2) == 2) ||(compare( typ, "ST", 2) == 2))
  {
    i=valuestack_ptr-1;
    if(i<0) printf(" stack is empty\n");
    else
    {
      j=0;
      while(0<=i)
      {
        printf ("%d %s\n",++j,valuestack[i]); i--;
      }
    }
  }
  else if ((compare( typ, "se", 2) == 2) ||(compare( typ, "SE", 2) == 2))
  {
    if (!anz->sets)
    {
      printf("ERROR: no sets defined\n");
      strcpy(parameter[0], "ERROR: no sets defined");
      write2stack(1, parameter);
    }
    if (length==1)
    {
      /* list all existing sets */
      for (i=0; i<anz->sets; i++)
      {
        if( set[i].name != (char *)NULL )
        {
          if (!set[i].type)
          {
            printf ("%-5d %s stat:%c n:%d e:%d f:%d p:%d l:%d s:%d b:%d L:%d S:%d se:%d sh:%d v:%d\n", set[i].index, set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh, set[i].anz_v);
          }
        }
      }
      return(1);
    }
    else
    {
      setNr=getSetNr(name);
      if((setNr>=0)&&(compare(param,"range",3)==3))
      {
        if((set[setNr].anz_n>2)&&(set[setNr].anz_e>2))
	{
          param[0]=0;
          /* check for holes */
          for(i=0; i<set[setNr].anz_n-1; i++)
            if((set[setNr].node[i]+1)!=(set[setNr].node[i+1])) break;
	  if(i<set[setNr].anz_n-1) printf("Warning set %s has holes in the node numbering starting between node %d and %d\n", set[setNr].name, set[setNr].node[i], set[setNr].node[i+1]);
          for(i=0; i<set[setNr].anz_e-1; i++)
            if((set[setNr].elem[i]+1)!=(set[setNr].elem[i+1])) break;
	  if(i<set[setNr].anz_e-1) printf("Warning set %s has holes in the elem numbering starting between elem %d and %d\n", set[setNr].name, set[setNr].elem[i], set[setNr].elem[i+1]);			       					       

          printf ("%-5d %s nr %d %d er %d %d\n", set[setNr].index, set[setNr].name, set[setNr].node[0], set[setNr].node[set[setNr].anz_n-1], set[setNr].elem[0], set[setNr].elem[set[setNr].anz_e-1]);
	}
        return(1);
      }

      if (setNr<0)
      {
        /* set was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
	  {
            foundSubString=0;
            for(i=0; i<length; i++)
	    {
              if(strstr(set[setNr].name, dat[i]) !=NULL)
	      {
	        foundSubString++;
	        /* check if the first or the last char is no '*' then the dat[] must be at start or end */
	        if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
         	if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	      }
	    }
            if(foundSubString==length)
	    {
	      i=setNr;
              if (!set[i].type)
              {
		j++;
                printf ("%-5d %s stat:%c n:%d e:%d f:%d p:%d l:%d s:%d b:%d L:%d S:%d se:%d sh:%d v:%d", set[i].index, set[i].name, set[i].flag, set[i].anz_n, set[i].anz_e, set[i].anz_f, set[i].anz_p, set[i].anz_l, set[i].anz_s, set[i].anz_b, set[i].anz_nurl, set[i].anz_nurs, set[i].anz_se, set[i].anz_sh, set[i].anz_v);
                if(compare(param,"range",3)==3)
		{
                  printf (" nr %d %d er %d %d", set[setNr].node[0], set[setNr].node[set[setNr].anz_n-1], set[setNr].elem[0], set[setNr].elem[set[setNr].anz_e-1]);
		}
                else printf ("\n");
              }
	    }
	  }
          if(j!=0) return(1);
	}
        /* free dat */
        for(i=0; i<length; i++) free(dat[i]);
        free(dat);

        printf ("ERROR: set %s does not exist\n", name);
        sprintf(parameter[0], "ERROR: set %s does not exist", name);
        write2stack(1, parameter);
        return (-1);
      }
    }
    if (set[setNr].type) return(0);

    if(anz->l)
    {
      printf (" node:    value:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%6e",lcase[cur_lc].dat[cur_entity][set[setNr].node[i]]);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[4],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4]);
        write2stack(5, parameter);
      }
    }
    else
    {
      printf (" node:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3]);
        write2stack(4, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_e; i++)
    {
      nr=set[setNr].elem[i];
      printf (" elem:%d type:%d n:", nr, e_enqire[nr].type);
      if     (e_enqire[nr].type==1) k=8;
      else if(e_enqire[nr].type==2) k=6;
      else if(e_enqire[nr].type==3) k=4;
      else if(e_enqire[nr].type==4) k=20;
      else if(e_enqire[nr].type==5) k=15;
      else if(e_enqire[nr].type==6) k=10;
      else if(e_enqire[nr].type==7) k=3;
      else if(e_enqire[nr].type==8) k=6;
      else if(e_enqire[nr].type==9) k=4;
      else if(e_enqire[nr].type==10) k=8;
      else if(e_enqire[nr].type==11) k=2;
      else if(e_enqire[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",e_enqire[nr].nod[n]);
      printf("\n"); 
    }
    for (i=0; i<set[setNr].anz_f; i++)
    {
      nr=set[setNr].face[i];
      printf (" face:%d elem:%d side:%d nodes:",  nr, face[nr].elem_nr, face[nr].nr+1);
      if(face[nr].type==7) k=3;
      else if(face[nr].type==8) k=6;
      else if(face[nr].type==9) k=4;
      else if(face[nr].type==10) k=8;
      else if(face[nr].type==11) k=2;
      else if(face[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",face[nr].nod[n]);
      printf("\n"); 
    }
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      if( point[p].name != (char *)NULL ) 
        printf (" pnt:%s %lf %lf %lf\n", point[p].name,
        (point[p].px* scale->w+scale->x),
        (point[p].py* scale->w+scale->y),
        (point[p].pz* scale->w+scale->z) );
      sprintf(parameter[0],"%s", point[p].name);
      sprintf(parameter[1],"%lf", point[p].px* scale->w+scale->x);
      sprintf(parameter[2],"%lf", point[p].py* scale->w+scale->y);
      sprintf(parameter[3],"%lf", point[p].pz* scale->w+scale->z);
      write2stack(4, parameter);
    }
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l=set[setNr].line[i];
      if( line[l].name != (char *)NULL ) 
      {
        bias_fbd=getBias_fbd(l, line);

        if( line[l].typ=='a' )  /* arc-line */
        {
          printf (" line:%s typ:a p1:%s p2:%s pc:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, point[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
          sprintf(parameter[4],"%s",point[line[l].trk].name);
        }
        else if( line[l].typ=='s' )  /* seq-line */
        {
          printf (" line:%s typ:s p1:%s p2:%s set:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, set[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
          sprintf(parameter[4],"%s",set[line[l].trk].name);
        }
        else  /* its a straight line   */
        {
          printf (" line:%s typ:l p1:%s p2:%s trk:- div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
          , line[l].name, point[line[l].p1].name,
          point[line[l].p2].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
          sprintf(parameter[4],"-");
        }
        sprintf(parameter[0],"%s",line[l].name);
        sprintf(parameter[1],"%c",line[l].typ);
        sprintf(parameter[2],"%s",point[line[l].p1].name);
        sprintf(parameter[3],"%s",point[line[l].p2].name);
        sprintf(parameter[5],"%d",line[l].div);
        sprintf(parameter[6],"%d",bias_fbd);
        sprintf(parameter[7],"%f",line[l].bias);
        sprintf(parameter[8],"%d",line[l].etyp);
        sprintf(parameter[9],"%d",line[l].eattr);
        write2stack(10, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_c; i++)
    {
      p=set[setNr].lcmb[i];
      if( lcmb[p].name != (char *)NULL ) 
      {
        printf (" lcmb:%s ", lcmb[p].name );
        for (j=0; j<lcmb[p].nl; j++)
          printf (" %1c %s", lcmb[p].o[j], line[lcmb[p].l[j]].name );
        printf (" \n");
      }
    }
    for (i=0; i<set[setNr].anz_s; i++)
    {
      p=set[setNr].surf[i];
      if( surf[p].name != (char *)NULL ) 
      {
	  // printf("sh:%d\n",surf[p].sh);
        if (surf[p].sh==-1) printf (" surf:%s %1c BLEND ", surf[p].name, surf[p].ori );
        else if (surf[p].sh>-1) printf (" surf:%s %1c %s ", surf[p].name, surf[p].ori, shape[surf[p].sh].name );
        for (j=0; j<surf[p].nl; j++)
        {
          nl=surf[p].l[j];
          if (surf[p].typ[j]=='l')
          {
            printf (" %1c %s", surf[p].o[j], line[nl].name );
          }
          else
          {
            printf (" %1c %s", surf[p].o[j], lcmb[nl].name );
            for (k=0; k<lcmb[nl].nl; k++ )
            {
              nc=lcmb[nl].l[k];
              printf (" (%c) (%s)", lcmb[nl].o[k], line[nc].name );
            }
          }
        }
        printf (" etyp:%d attr:%d\n", surf[p].etyp, surf[p].eattr);
      }
    }
    for (i=0; i<set[setNr].anz_b; i++)
    {
      p=set[setNr].body[i];
      if( body[p].name != (char *)NULL ) 
      {
        printf (" body:%s %1c", body[p].name, body[p].ori );
        for (j=0; j<body[p].ns; j++)
          printf (" %1c %s", body[p].o[j], surf[body[p].s[j]].name );
        printf (" etyp:%d attr:%d\n", body[p].etyp, body[p].eattr);
      }
    }
    for (i=0; i<set[setNr].anz_nurl; i++)
    {
      nr=set[setNr].nurl[i];
      if( nurbl[nr].name != (char *)NULL ) 
      {
        printf (" nurl:%s\n", nurbl[nr].name );
        printf (" coefficients: exp:%d pnts:%d nods:%d End:%d\n", nurbl[nr].u_exp, nurbl[nr].u_npnt, nurbl[nr].u_nknt, nurbl[nr].endFlag);
        for (k=0; k<nurbl[nr].u_nknt; k++) printf("u-node[%d] = %lf\n", k+1, nurbl[nr].uknt[k]);
        for (k=0; k<nurbl[nr].u_npnt; k++)
        {
          printf("cpnt[%d]:%s x=%lf y=%lf z=%lf w=%lf\n", k+1, point[nurbl[nr].ctlpnt[k]].name, point[nurbl[nr].ctlpnt[k]].px* scale->w+scale->x,
          point[nurbl[nr].ctlpnt[k]].py* scale->w+scale->y, point[nurbl[nr].ctlpnt[k]].pz* scale->w+scale->z, nurbl[nr].weight[k]);
        }
      }
    }
    for (i=0; i<set[setNr].anz_nurs; i++)
    {
      nr=set[setNr].nurs[i];
     if( nurbs[nr].name != (char *)NULL ) 
     {
      printf (" nurs:%s\n", nurbs[nr].name );

      printf (" coefficients: u_exp:%d v_exp:%d u_npnt:%d v_npnt:%d u_nnod:%d v_nnod:%d End:%d\n", nurbs[nr].u_exp, nurbs[nr].v_exp,
        nurbs[nr].u_npnt, nurbs[nr].v_npnt, nurbs[nr].u_nknt, nurbs[nr].v_nknt, nurbs[nr].endFlag);
      for (k=0; k<nurbs[nr].u_nknt; k++) printf("u-node[%d] = %lf\n", k+1, nurbs[nr].uknt[k]);
      for (k=0; k<nurbs[nr].v_nknt; k++) printf("v-node[%d] = %lf\n", k+1, nurbs[nr].vknt[k]);
      for (k=0; k<nurbs[nr].u_npnt; k++)
      {
        for (j=0; j<nurbs[nr].v_npnt; j++)
        {
          printf("cpnt[%d][%d]:%s x=%lf y=%lf z=%lf w=%lf \n", k+1, j+1, point[nurbs[nr].ctlpnt[k][j]].name, point[nurbs[nr].ctlpnt[k][j]].px* scale->w+scale->x,
          point[nurbs[nr].ctlpnt[k][j]].py* scale->w+scale->y, point[nurbs[nr].ctlpnt[k][j]].pz* scale->w+scale->z, nurbs[nr].weight[k][j]);
        }
      }
     }
    }
    for (i=0; i<set[setNr].anz_se; i++)
    {
      p=set[setNr].set[i];
      if( set[p].name != (char *)NULL ) 
      {
        printf (" set:%s ", set[p].name );
        printf (" \n");
      }
    }
    for (i=0; i<set[setNr].anz_sh; i++)
    {
      p=set[setNr].shp[i];
      if( shape[p].name != (char *)NULL ) 
      {
        if(shape[p].type==0)
          printf (" shape:%s PLN p1:%s p2:%s p3:%s\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, point[shape[p].p[2]].name );
        if(shape[p].type==1)
          printf (" shape:%s CYL p1:%s p2:%s p3:%s\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, point[shape[p].p[2]].name );
        if(shape[p].type==2)
          printf (" shape:%s CONE p1:%s p2:%s p3:%s p4:%s\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name, point[shape[p].p[2]].name, point[shape[p].p[3]].name );
        if(shape[p].type==3)
          printf (" shape:%s SPH p1:%s p2:%s\n", shape[p].name, point[shape[p].p[0]].name, point[shape[p].p[1]].name );
        if(shape[p].type==4)
          printf (" shape:%s NURS %s\n", shape[p].name, nurbs[shape[p].p[0]].name);
      }
    }
    for (i=0; i<set[setNr].anz_v; i++)
    {
      p=set[setNr].valu[i];
      if( value[p].name != (char *)NULL ) 
      {
        printf (" value:%s string:%s\n", value[p].name, value[p].string);
      }
    }
    printf (" %d nodes, %d elements, %d faces, %d Points, %d Lines, %d Lcmb, %d Surfs, %d Bodys, %d Nurl, %d Nurs, %d sets, %d shapes, %d values stored\n", set[setNr].anz_n, set[setNr].anz_e, set[setNr].anz_f, set[setNr].anz_p, set[setNr].anz_l, set[setNr].anz_c, set[setNr].anz_s, set[setNr].anz_b, set[setNr].anz_nurl, set[setNr].anz_nurs, set[setNr].anz_se, set[setNr].anz_sh, set[setNr].anz_v );
  }

  else if ((compare( typ, "sh", 2) == 2) ||(compare( typ, "SH", 2) == 2))
  {
    l=getShapeNr(name);
    if (l<0)
    {
      printf (" prnt: shape:%s does not exist\n", name);
      return (-1);
    }
    if( shape[l].name != (char *)NULL ) 
    {
      printf (" shape:%s p1:%s p2:%s p3:%s\n", shape[l].name, point[shape[l].p[0]].name, point[shape[l].p[1]].name, point[shape[l].p[2]].name);
    }
  }
  else if ((compare( typ, "sq", 2) == 2) ||(compare( typ, "SQ", 2) == 2))
  {
    if (length==1)
    {
      /* list all existing sequential sets */
      l=0;
      for (i=0; i<anz->sets; i++)
      {
        if( set[i].name != (char *)NULL )
        {
          if (set[i].type)
	  {
            l++;
            printf (" %s type:SEQ stat:%c n:%d p:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_p);
	  }
        }
      }
      if (!l) printf(" no sequences defined\n");
      return(1);
    }
    else 
    {
      setNr=getSetNr(name);
      if (setNr<0)
      {
        /* set was not found. check if wildcards (*) were used */
        length= strsplt( name, '*', &dat);
        if ((length>0)&&(strstr(name, "*") !=NULL))
	{
          j=0;
          for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
	  {
            foundSubString=0;
            for(i=0; i<length; i++)
            {
              if(strstr(set[setNr].name, dat[i]) !=NULL)
	      {
	        foundSubString++;
	        /* check if the first or the last char is no '*' then the dat[] must be at start or end */
	        if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
         	if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	      }
	    }
            if(foundSubString==length)
	    {
	      i=setNr;
              if (set[i].type)
              {
                printf ("%s type:SEQ stat:%c n:%d p:%d\n", set[i].name, set[i].flag, set[i].anz_n, set[i].anz_p);
		j++;
              }
	    }
	  }
          if(j!=0) return(1);
	}
        printf (" prnt: seq:%s does not exist\n", name);
        return(-1);
      }
    }
    if (!set[setNr].type) return(0);

    if(anz->l)
    {
      printf (" node:    value:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%6e",lcase[cur_lc].dat[cur_entity][set[setNr].node[i]]);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[4],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4]);
        write2stack(5, parameter);
      }
    }
    else
    {
      printf (" node:       x:            y:           z:\n");
      for (i=0; i<set[setNr].anz_n; i++)
      {
        sprintf(parameter[0],"%8d",set[setNr].node[i]);
        sprintf(parameter[1],"%12f",node[set[setNr].node[i]].nx* scale->w+scale->x);
        sprintf(parameter[2],"%12f",node[set[setNr].node[i]].ny* scale->w+scale->y);
        sprintf(parameter[3],"%12f",node[set[setNr].node[i]].nz* scale->w+scale->z);
        printf (" %s %s %s %s\n",parameter[0],parameter[1],parameter[2],parameter[3]);
        write2stack(4, parameter);
      }
    }
    for (i=0; i<set[setNr].anz_p; i++)
    {
      p=set[setNr].pnt[i];
      if( point[p].name != (char *)NULL ) 
        printf (" pnt:%s x:%lf y:%lf z:%lf\n", point[p].name,
        (point[p].px* scale->w+scale->x),
        (point[p].py* scale->w+scale->y),
        (point[p].pz* scale->w+scale->z) );
    }
  }


  else if ((typ[0]=='n')||(typ[0]=='N'))
  {
    nr=atoi(name);
    if(anz->l)
    {
      printf (" node:%d v:%lf x:%lf y:%lf z:%lf\n", nr, lcase[cur_lc].dat[cur_entity][nr],
        (node[nr].nx* scale->w+scale->x),
        (node[nr].ny* scale->w+scale->y),
        (node[nr].nz* scale->w+scale->z) );
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%lf",lcase[cur_lc].dat[cur_entity][nr]);
      sprintf(parameter[1],"%lf",node[nr].nx * scale->w+scale->x);
      sprintf(parameter[2],"%lf",node[nr].ny * scale->w+scale->y);
      sprintf(parameter[3],"%lf",node[nr].nz * scale->w+scale->z);
      write2stack(5, parameter);
    }
    else
    {
      printf (" node:%d xyz: %lf %lf %lf\n", nr,
        (node[nr].nx* scale->w+scale->x),
        (node[nr].ny* scale->w+scale->y),
        (node[nr].nz* scale->w+scale->z) );
      sprintf(parameter[0],"%d", nr);
      sprintf(parameter[1],"%lf",node[nr].nx * scale->w+scale->x);
      sprintf(parameter[2],"%lf",node[nr].ny * scale->w+scale->y);
      sprintf(parameter[3],"%lf",node[nr].nz * scale->w+scale->z);
      write2stack(4, parameter);
    }
  }
  else if ((typ[0]=='e')||(typ[0]=='E'))
  {
    nr=atoi(name);
    printf (" elem:%d ",  nr);
    if     (e_enqire[nr].type==1) k=8;
    else if(e_enqire[nr].type==2) k=6;
    else if(e_enqire[nr].type==3) k=4;
    else if(e_enqire[nr].type==4) k=20;
    else if(e_enqire[nr].type==5) k=15;
    else if(e_enqire[nr].type==6) k=10;
    else if(e_enqire[nr].type==7) k=3;
    else if(e_enqire[nr].type==8) k=6;
    else if(e_enqire[nr].type==9) k=4;
    else if(e_enqire[nr].type==10) k=8;
    else if(e_enqire[nr].type==11) k=2;
    else if(e_enqire[nr].type==12) k=3;
    else k=0;
    for (n=0; n<k; n++) printf("%d ",e_enqire[nr].nod[n]);
    printf("\n"); 
  }
  else if ((typ[0]=='f')||(typ[0]=='F'))
  {
    nr=atoi(name);
    printf (" face:%d elem:%d side:%d nodes:",  nr, face[nr].elem_nr, face[nr].nr+1);
      if(face[nr].type==7) k=3;
      else if(face[nr].type==8) k=6;
      else if(face[nr].type==9) k=4;
      else if(face[nr].type==10) k=8;
      else if(face[nr].type==11) k=2;
      else if(face[nr].type==12) k=3;
      else k=0;
      for (n=0; n<k; n++) printf("%d ",face[nr].nod[n]);
      printf("\n"); 
  }
  else if ((typ[0]=='p')||(typ[0]=='P'))
  {
    p=getPntNr(name);
    if (p<0)
    {
      printf (" prnt: point:%s does not exist\n", name);
      return (-1);
    }
    if( point[p].name != (char *)NULL )
    {
      printf (" pnt:%s x:%lf y:%lf z:%lf\n", point[p].name,
      (point[p].px* scale->w+scale->x),
      (point[p].py* scale->w+scale->y),
      (point[p].pz* scale->w+scale->z) );
      sprintf(parameter[0],"%s", point[p].name);
      sprintf(parameter[1],"%lf", point[p].px* scale->w+scale->x);
      sprintf(parameter[2],"%lf", point[p].py* scale->w+scale->y);
      sprintf(parameter[3],"%lf", point[p].pz* scale->w+scale->z);
      write2stack(4, parameter);
    }
  }
  else if ((typ[0]=='l')||(typ[0]=='L'))
  {
    l=getLineNr(name);
    if (l<0)
    {
      printf (" prnt: line:%s does not exist\n", name);
      return (-1);
    }
    if( line[l].name != (char *)NULL ) 
    {
      bias_fbd=getBias_fbd(l, line);

      if( line[l].typ=='a' )  /* arc-line */
      {
        printf (" line:%s p1:%s p2:%s pc:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, point[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
      }
      else if( line[l].typ=='s' )  /* seq-line */
      {
        printf (" line:%s p1:%s p2:%s set:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, set[line[l].trk].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
      }
      else  /* its a straight line   */
      {
        printf (" line:%s p1:%s p2:%s div:%d bias:%d bias_el:%lf etyp:%d attr:%d\n"
        , line[l].name, point[line[l].p1].name,
        point[line[l].p2].name, line[l].div, bias_fbd, line[l].bias, line[l].etyp, line[l].eattr);
      }
    }
  }
  else if ((typ[0]=='c')||(typ[0]=='C'))
  {
    l=getLcmbNr(name);
    if (l<0)
    {
      printf (" prnt: lcmb:%s does not exist\n", name);
      return (-1);
    }  
    if( lcmb[l].name != (char *)NULL ) 
    {
      printf (" lcmb:%s ", lcmb[l].name );
      for (j=0; j<lcmb[l].nl; j++)
        printf (" %1c %s", lcmb[l].o[j], line[lcmb[l].l[j]].name );
      printf (" \n");
    }
  }
  else if ((typ[0]=='s')||(typ[0]=='S'))
  {
    p=getSurfNr(name);
    if (p<0)
    {
      printf (" prnt: surf:%s does not exist\n", name);
      return (-1);
    }  
    if( surf[p].name != (char *)NULL ) 
    {
      if (surf[p].sh==-1) printf (" surf:%s %1c BLEND ", surf[p].name, surf[p].ori );
      else if (surf[p].sh>-1) printf (" surf:%s %1c %s ", surf[p].name, surf[p].ori, shape[surf[p].sh].name );
      for (j=0; j<surf[p].nl; j++)
      {
        nl=surf[p].l[j];
        if (surf[p].typ[j]=='l')
        {
          printf (" %1c %s", surf[p].o[j], line[nl].name );
        }
        else
        {
          printf (" %1c %s", surf[p].o[j], lcmb[nl].name );
          for (k=0; k<lcmb[nl].nl; k++ )
          {
            nc=lcmb[nl].l[k];
            printf (" (%c) (%s)", lcmb[nl].o[k], line[nc].name );
          }
        }
      }
      printf (" etyp:%d attr:%d\n", surf[p].etyp, surf[p].eattr);
    }
  }
  else if ((typ[0]=='b')||(typ[0]=='B'))
  {
    p=getBodyNr(name);
    if (p<0)
    {
      printf (" prnt: body:%s does not exist\n", name);
      return (-1);
    }  
    if( body[p].name != (char *)NULL ) 
    {
      printf (" body:%s %1c", body[p].name, body[p].ori );
      for (j=0; j<body[p].ns; j++)
        printf (" %1c %s", body[p].o[j], surf[body[p].s[j]].name );
      printf (" etyp:%d attr:%d\n", body[p].etyp, body[p].eattr);
    }
  }
  else if ((typ[0]=='v')||(typ[0]=='V'))
  {
    if (length==1)
    {
      /* list all existing values */
      for (i=0; i<anz->v; i++)
      {
        if( value[i].name != (char *)NULL )
        {
          printf (" value %s stores %s\n", value[i].name, value[i].string );
        }
      }
      return(1);
    }
    p=getValuNr(name);
    if (p<0)
    {
      printf (" prnt: value:%s does not exist\n", name);
      return (-1);
    }  
    if( value[p].name != (char *)NULL ) 
    {
      printf (" value %s stores %s\n", value[p].name, value[p].string );

      /* split the value[p].string in separate strings at ' ' occurences and write them to the stack */
      strcpy(param,value[p].string);
      for(args=0, str=param; ; args++, str=NULL)
      {
        token = strtok_r(str," ", &saveptr);
        if(token == NULL) break;
        if(args == 20) break;
        strcpy(parameter[args],token);
      }
      write2stack(args, parameter);
    }
  }
  return(1);
}

/*------------------------------------------------------------------*/
/* set und setinhalt loeschen                                       */
/*------------------------------------------------------------------*/

int zap(char *record)
{
  int i,j;
  int setNr;
  int addDispFlagLocal=0;
  char setname[MAX_LINE_LENGTH];

  sword( record, setname );
  operateAlias( setname, "se" );
  setNr=getSetNr(setname);

  if (setNr<0) return (-1);

  /* suche abhaengige linien, redefine if necessary */
  for ( i=0; i<set[setNr].anz_p; i++)
  {
    for (j=0; j<anzGeo->l; j++)
    {
      if(( set[setNr].pnt[i] == line[j].trk )&&( line[j].typ == 'a' ))
      {
        line[j].typ = ' ';
      }
    }
  }

  /* vor dem loeschen muessen noch die abhaengigen direkt uebergeordneten Groessen dazugeladen werden  */
  printf(" complete set\n");
  completeSet( setname, "up");
  printf(" set complete\n");

  if (set[setNr].anz_n>0)
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    delNod( set[setNr].anz_n, set[setNr].node );
    printf(" nodes deleted\n");
  }
  if (set[setNr].anz_e>0)
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    delElem( set[setNr].anz_e, set[setNr].elem );
    printf(" elements deleted\n");
  }
  if(addDispFlagLocal==2) { addDispToCoordinates(node); }
  if (set[setNr].anz_nurs>0)
  {
    delNurs( set[setNr].anz_nurs, set[setNr].nurs );
    printf(" nurs deleted\n");
  }
  if (set[setNr].anz_sh>0)
  {
    delShape( set[setNr].anz_sh, set[setNr].shp );
    printf(" shapes deleted\n");
  }
  if (set[setNr].anz_b>0)
  {
    delBody( set[setNr].anz_b, set[setNr].body );
    printf(" bodys deleted\n");
  }
  if (set[setNr].anz_s>0)
  {
    delSurf( set[setNr].anz_s, set[setNr].surf );
    printf(" surfs deleted\n");
  }
  if (set[setNr].anz_c>0)
  {
    delLcmb( set[setNr].anz_c, set[setNr].lcmb );
    printf(" lcmbs deleted\n");
  }
  if (set[setNr].anz_l>0)
  {
    delLine( set[setNr].anz_l, set[setNr].line );
    printf(" lines deleted\n");
  }
  if (set[setNr].anz_p>0)
  {
    delPnt( set[setNr].anz_p, set[setNr].pnt );
    printf(" points deleted\n");
  }
  if (set[setNr].anz_v>0)
  {
    delVal( set[setNr].anz_v, set[setNr].valu );
    printf(" values deleted\n");
  }

  /* delete the set itself */
  delSet(setname);

  return(1);
}



void mata( int *elemMat, int mat, int setNr )
{
  register int  i, j;
  int           *ielem=NULL;

  /* mark the elements of the set for easy identification */
  if( (ielem=(int *)malloc((anz->emax+1)*sizeof(int) ) )==NULL) 
  { printf(" ERROR: malloc failure\n"); return; }
  if (setNr==-1)
    for (i=0; i<=anz->emax; i++) ielem[i]=1;
  else
  {
    for (i=0; i<=anz->emax; i++) ielem[i]=0;
    for (i=0; i<set[setNr].anz_e; i++) ielem[set[setNr].elem[i]]=1;
  }

  /* exists this mat? */
  for (i=1; i<=elemMat[0]; i++) if (elemMat[i]==mat) break;
  if(i>elemMat[0])
  {
    elemMat[0] = i;
    elemMat[i] = mat;
  }
  
  for (j=0; j<anz->e; j++)
  {
    if (ielem[e_enqire[j].nr])
      e_enqire[e_enqire[j].nr].mat= elemMat[i];
  }
  free(ielem); ielem=NULL;
}


char getLetter(int i)
{
  if(i>9) return(i-10+65);
  else return(i+48);
}


int getFamName( int n, char *c )
{
  int i, j;
  double f;

  j=n/46656;
  n=n%46656;

  /* change decimal (0->46655) into a string of 3 char (000 -> ZZZ) */
  f=n/1296.;
  i=f+1e-10;
  c[0]=getLetter(i);
  f=(f-i)*36.;
  i=f+1e-10;
  c[1]=getLetter(i);
  i=(f-i)*36.+1e-10;
  c[2]=getLetter(i);
  c[3]='\0';
  return(j);
}


int getNewName( char *name, char *type )
{
  static int p, l, c, s, b, L, S, se, sh;
  int i,nr,nr2=0;
  char string[MAX_LINE_LENGTH];

  /* not implemented yet 
  if(fillSpaceFlag) p=l=c=s=b=S=se=0;
  */

    do
    {
      if (type[0]=='p')
      {
        p++;
        i=getFamName(p, string);
        if(lchar->p+i>90)  sprintf(name, "%c%-d", lchar->p, p );
        else sprintf(name, "%c%s", lchar->p+i, string );
        nr=getPntNr(name);
      }
      else if (compare( type, "se", 2) == 2 )
      {
        se++;
        i=getFamName(se, string);
        if(lchar->se+i>90)  sprintf(name, "%c%-d", lchar->se,se);
        else sprintf(name, "%c%s", lchar->se+i, string );
        nr=getSetNr(name);
      }
      else if (compare( type, "sh", 2) == 2 )
      {
        sh++;
        i=getFamName(sh, string);
        if(lchar->sh+i>90)  sprintf(name, "%c%-d", lchar->sh,sh);
        else sprintf(name, "%c%s", lchar->sh+i, string );
        nr=getShapeNr(name);
      }
      else if (type[0]=='l')
      {
        l++;
        i=getFamName(l, string);
        if(lchar->l+i>90)  sprintf(name, "%c%-d", lchar->l, l );
        else sprintf(name, "%c%s", lchar->l+i, string );
        nr=getLineNr(name);
        if(nr<0) nr2=getLcmbNr(name); if(nr2>0) nr=0; /* check if a lcmb has this name, not use it */
      }
      else if (type[0]=='c')
      {
        c++;
        i=getFamName(c, string);
        if(lchar->c+i>90)  sprintf(name, "%c%-d", lchar->c, c );
        else sprintf(name, "%c%s", lchar->c+i, string );
        nr=getLcmbNr(name);
        if(nr<0) nr2=getLineNr(name); if(nr2>0) nr=0; /* check if a line has this name, not use it */
      }
      else if (type[0]=='s')
      {
        s++;
        i=getFamName(s, string);
        if(lchar->s+i>90)  sprintf(name, "%c%-d", lchar->s, s );
        else sprintf(name, "%c%s", lchar->s+i, string );
        nr=getSurfNr(name);
      }
      else if (type[0]=='b')
      {
        b++;
        i=getFamName(b, string);
        if(lchar->b+i>90)  sprintf(name, "%c%-d", lchar->b, b );
        else sprintf(name, "%c%s", lchar->b+i, string );
        nr=getBodyNr(name);
      }
      else if (type[0]=='L')
      {
        L++;
        i=getFamName(L, string);
        if(lchar->L+i>90)  sprintf(name, "%c%-d", lchar->L, L );
        else sprintf(name, "%c%s", lchar->L+i, string );
        nr=getNurlNr(name);
      }
      else if (type[0]=='S')
      {
        S++;
        i=getFamName(S, string);
        if(lchar->S+i>90)  sprintf(name, "%c%-d", lchar->S, S );
        else sprintf(name, "%c%s", lchar->S+i, string );
        nr=getNursNr(name);
      }
      else
        return(-1);
    }while(nr>-1);   /* new entity */

  return(1);
}



double calcGTOL(int setNr)
{
  int l,i,n;
  double max[3], min[3];

  /* calculate the geometric tolerance based on all line-end-points */
  for(i=0; i<3; i++)
  {
    max[i]=-MAX_INTEGER;
    min[i]= MAX_INTEGER;
  }
  for(l=0; l<set[setNr].anz_l; l++)
  {
    i=set[setNr].line[l];
    if(line[i].name!=(char *)NULL)
    {
      if(point[line[i].p1].px > max[0]) max[0]=point[line[i].p1].px;
      if(point[line[i].p2].px > max[0]) max[0]=point[line[i].p2].px;
      if(point[line[i].p1].py > max[1]) max[1]=point[line[i].p1].py;
      if(point[line[i].p2].py > max[1]) max[1]=point[line[i].p2].py;
      if(point[line[i].p1].pz > max[2]) max[2]=point[line[i].p1].pz;
      if(point[line[i].p2].pz > max[2]) max[2]=point[line[i].p2].pz;
  
      if(point[line[i].p1].px < min[0]) min[0]=point[line[i].p1].px;
      if(point[line[i].p2].px < min[0]) min[0]=point[line[i].p2].px;
      if(point[line[i].p1].py < min[1]) min[1]=point[line[i].p1].py;
      if(point[line[i].p2].py < min[1]) min[1]=point[line[i].p2].py;
      if(point[line[i].p1].pz < min[2]) min[2]=point[line[i].p1].pz;
      if(point[line[i].p2].pz < min[2]) min[2]=point[line[i].p2].pz;
    }
  }
  gtol=GTOL;
  for(i=0; i<3; i++)
  {
    max[i]-=min[i];
    if(max[i]>gtol) gtol=max[i];
  }
  gtol=GTOL*gtol*scale->w;

  if(valuestackFlag)
  {
    n=1;
    if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+n)*sizeof(char *)) ) == NULL )
    { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    for(i=0; i<n; i++)
    {
      if ((valuestack[valuestack_ptr+i] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return(-1); }
    }
    sprintf(valuestack[valuestack_ptr++],"%f", gtol );
    printf(" gtol written to stack\n");
  }

  return(gtol);
}



int readlist(char *datin, char *type)
{
  FILE *handle;

  int length, i, j, n, dotpos, column, setNr;
  char string[MAX_LINE_LENGTH], face[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH];
  int nr;

  handle = fopen (datin, "r");
  if (handle==NULL)
  {
    printf (" ERROR: The input file \"%s\" could not be opened.\n\n", datin);
    return(-1);
  }
  else  printf ("\n%s opened",datin);

  length = 1;
  printf ("\n reading file\n");

  i=n=0;
  dotpos = strlen (datin);
  for (j=0; j<dotpos; j++) if (datin[j]=='/') n=j+1;
  for (j=n; j<dotpos; j++)
  {
    if (datin[j]=='.')
    {
       break;
    }
    else datin[i]=datin[j];
    i++;
  }
  datin[i]= '\0';

  /* initialize the set */
  if( (setNr=pre_seta( datin, "is", 0)) <0 ) return(-1);
 
  /* get the column */
  if(strlen(type)>1) column=atoi(&type[1])-1;
  else column=0;

  if((type[0]=='E')||(type[0]=='e'))
  {
    for (i=0; i<column; i++) sprintf(&format[i*4], "%%*s ");
    sprintf(&format[i*4], "%%d, %%s");
    while (length > -1)
    {
      length = frecord( handle, string);
      if( string[length]== (char)EOF)  break;
      n=sscanf(string, format, &nr, face );
      printf("rec:%d e:%d face:%s\n",n,nr,face);
      if(n)
      {
        printf (" seta %s e %d\n", datin, nr );
        seta( setNr, "e", nr );
        if(n==2) /* element face is provided */
	{
          if(e_enqire[nr].type==1)
          {
	    if(face[1]=='1')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
	    }
	    else if(face[1]=='2')
            {
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
	    else if(face[1]=='3')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
	    }
	    else if(face[1]=='4')
            {
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
	    }
	    else if(face[1]=='5')
            {
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
	    }
	    else if(face[1]=='6')
            {
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
            else printf(" side:%s not supported\n", face);
	  }
          else if(e_enqire[nr].type==6)
          {
	    if(face[1]=='1')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
	    }
	    else if(face[1]=='2')
            {
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[4] );
              seta( setNr, "n", e_enqire[nr].nod[8] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
	    }
	    else if(face[1]=='3')
            {
              seta( setNr, "n", e_enqire[nr].nod[1] );
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[5] );
              seta( setNr, "n", e_enqire[nr].nod[9] );
              seta( setNr, "n", e_enqire[nr].nod[8] );
	    }
	    else if(face[1]=='4')
            {
              seta( setNr, "n", e_enqire[nr].nod[2] );
              seta( setNr, "n", e_enqire[nr].nod[0] );
              seta( setNr, "n", e_enqire[nr].nod[3] );
              seta( setNr, "n", e_enqire[nr].nod[6] );
              seta( setNr, "n", e_enqire[nr].nod[7] );
              seta( setNr, "n", e_enqire[nr].nod[9] );
	    }
            else printf(" side:%s not supported\n", face);
	  }
          else printf(" element type:%d not supported\n", e_enqire[nr].type);
	}
      }
    }
  }
  else if((type[0]=='N')||(type[0]=='n'))
  {
    for (i=0; i<column; i++) sprintf(&format[i*4], "%%*s ");
    sprintf(&format[i*4], "%%d");
    while (length > -1)
    {
      length = frecord( handle, string);
      if( string[length]== (char)EOF)  break;
      n=sscanf((const char *)string, (const char *)format, &nr );
      printf("rec:%d n:%d\n",n,nr);
      if(n)
      {
        printf (" seta %s n %d\n", datin, nr );
        seta( setNr, "n", nr );
      }
    }
  }
  else printf (" ERROR: type %s not known\n", type );
  fclose(handle);
  printf ("\n A sequence of name:%s was created. Please use 'prnt sq' for listings \n\n", set[setNr].name );
  return(1);
}


void pre_norm(char *name)
{
  int i,j, setNr, setcopy;
  static int *sum_n=NULL;
  static Nodes *norm=NULL;

  setNr=getSetNr(name);
  if (setNr<0)
  {
    printf (" ERROR: set:%s does not exist\n", name);
    return;
  }

  /* determine the normal based on all connected faces */
  setcopy=pre_seta( "+norm", "i", 0);
  if (setcopy<0)
  {
    errMsg (" ERROR: set:+norm could not be created\n" );
    return;
  }
  for (i=0; i<set[setNr].anz_n; i++)
  {
    seta( setcopy, "n", set[setNr].node[i] );
  }
  completeSet("+norm", "up") ;
  completeSet("+norm", "do") ;
  getNodeNormalen(&sum_n, &norm, setcopy, anz, face);
  delSet("+norm");

  for(i=0; i<set[setNr].anz_n; i++)
  {
    printf("node:%d norm:%f %f %f\n",set[setNr].node[i], norm[set[setNr].node[i]].nx, norm[set[setNr].node[i]].ny ,norm[set[setNr].node[i]].nz);
    if(valuestackFlag)
    {
      if ((valuestack = (char **)realloc( (char **)valuestack, (valuestack_ptr+4)*sizeof(char *)) ) == NULL )
      { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      for(j=0; j<4; j++)
      {
        if ((valuestack[valuestack_ptr+j] = (char *)malloc( MAX_LINE_LENGTH*sizeof(char)) ) == NULL )
        { printf("\n\nERROR: realloc failure, valuestack\n\n"); return; }
      }
      sprintf(valuestack[valuestack_ptr++],"%f", norm[set[setNr].node[i]].nz );
      sprintf(valuestack[valuestack_ptr++],"%f", norm[set[setNr].node[i]].ny );
      sprintf(valuestack[valuestack_ptr++],"%f", norm[set[setNr].node[i]].nx );
      sprintf(valuestack[valuestack_ptr++],"%d", set[setNr].node[i] );
      if(valuestackFlag) printf(" 4 values in inverse order written to stack\n");
    }
  }
}

void pre_movie(char *string)
{
  int i,j,k, length;
  static int delay=10;
  double val1=0, val2=0;
  char type[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], movie[MAX_LINE_LENGTH];
  movie[0]=0;
  length=sscanf(string, "%s %lf %lf %s", type, &val1, &val2, movie );

  if (compareStrings(type, "delay")>0) { if(length==2) delay=100.*val1; else delay=10.; }
  if (compareStrings(type, "start")>0) { movieFlag=1; stopFlag=0; }
  if (compareStrings(type, "stop")>0)
  {
    printf("movie stopped, make movie with 'movie make <nr> <nr> [movie]'\n");
    printf("optionally define the delay-time between pictures with:'movie delay <sec>' before 'movi make'\n");
    printf("optionally reset the counter and delete the single frames with 'movi clean'\n");
    printf("you might use the program 'realplay' or 'firefox' to play the movie.gif file\n");
    movieFlag=0;
    sprintf( buffer, "rm -f  hcpy_0.tga %s", DEV_NULL2);
    system (buffer);
  }
  if (compareStrings(type, "frames")>0)
  {
    sprintf( buffer, "rm -f _*.gif %s", DEV_NULL2);
    system (buffer);
    gifNr=1;
    movieFlag=1;
    stopFlag=0;
    length=sscanf(string, "%*s %s %s", name, movie);
    if((length==1)&&(name[0]=='a'))
    { movieFrames=-1; movieCommandFile[0]=0; }
    else
    {
      movieFrames=(int)val1;
      length=sscanf(string, "%*s %*s %s", movieCommandFile);
      if(length<1) movieCommandFile[0]=0;
    }
    //printf("movieFrames:%d movieCommandFile:%s\n",movieCommandFile);
  }
  if (compareStrings(type, "clean")>0)
  {
    sprintf( buffer, "rm -f _*.gif %s", DEV_NULL2);
    system (buffer);
    gifNr=1;
  }
  if (compareStrings(type, "make")>0)
  {
    if(length==1) {  val2=gifNr-1, val1=1; length=3; }  
    else if(length==2) {  val2=val1, val1=1; length=3; }  

    if(length>2)
    {
      if(length==4)
      {
        printf("make movie from %s and pic:%d to %d, wait for ready\n", movie, (int)val1,(int)val2);
        sprintf( buffer, "cp %s movie.gif %s", movie, DEV_NULL2);
        system (buffer);
      }
      else
      {
        printf("make movie from pic:%d to %d, wait for ready\n", (int)val1,(int)val2);
        movie[0]=0;
      }
        
      /* generate movie from single gif files */
      j=k=0;
      for(i=(int)val1; i<=(int)val2; i++)
      { 
        sprintf(&name[j*10], "_%d.gif     ",i); j++;
        if(j==9)
        {
  	j=0;
          sprintf( buffer, "convert -delay %d %s __%d.gif %s", delay, name, k++,DEV_NULL2);
          system (buffer);
        }
      }
      if(j)
      {
        sprintf( buffer, "convert -delay %d %s __%d.gif %s", delay, name, k++,DEV_NULL2);
        system (buffer);
      }
  
      /* assemble all movies */
      j=0;
      for(i=0; i<k; i++)
      { 
        sprintf(&name[j*12], "__%d.gif     ",i); j++;
        if(j==9)
        {
  	  j=0;
          if (i==9) sprintf( buffer, "convert -delay %d %s %s movie.gif %s", delay, movie, name,DEV_NULL2);
          else sprintf( buffer, "convert -delay %d movie.gif %s movie.gif %s", delay, name,DEV_NULL2);
          system (buffer);
        }
      }
      if(j)
      {
        if (k<9) sprintf( buffer, "convert -delay %d %s %s movie.gif %s", delay, movie, name,DEV_NULL2);
        else sprintf( buffer, "convert -delay %d movie.gif %s movie.gif %s", delay, name,DEV_NULL2);
        system (buffer);
      }
      sprintf( buffer, "rm __*.gif %s", DEV_NULL2);
      system (buffer);
      printf("\nready\n");
      printf("\nyou might use the program 'realplay' or 'firefox' to play the movie.gif file\n\n");
    }
    else
    {
      printf(" ERROR: make movie with 'movie make <nr> <nr> [movie]'\n");
    }
  }
}



int calcCoefficientsTet(int nslav, int *emas, int n_closest_tets, Nodes *node, Tetraeder *tet, double cof[4], int extrapolflag)
{
  int i,e=0;
  int secondLoopFlag=0;
  double v12[3], v13[3], v14[3], vn[3], sum=0.;
  int closestElem=0;
  double closestDist=-MAX_FLOAT;

  /* ----------------------------------------- */
  //int j, el=1;
  //FILE *handle;
  /* ----------------------------------------- */

  for(e=0; e<n_closest_tets; e++)
  {
    secondLoop:;
    /* ----------------------------------------- */
    /*
    handle = fopen ( "int.inp", "a");
    fprintf(handle,"*NODE, NSET=Nall\n");
    j=nslav;
    fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
    for (i=0; i<4; i++)
    {
      j=tet[emas[e]].n[i];
      fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
    }
    fprintf(handle,"*ELEMENTS, TYPE=C3D4\n");
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[0], tet[emas[e]].n[1], tet[emas[e]].n[2], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[1], tet[emas[e]].n[3], tet[emas[e]].n[2], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[2], tet[emas[e]].n[3], tet[emas[e]].n[0], nslav);
    fprintf(handle," %d,%d,%d,%d,%d\n", el++, tet[emas[e]].n[3], tet[emas[e]].n[1], tet[emas[e]].n[0], nslav);
    fclose(handle);
    */
    /* ----------------------------------------- */

    /* calc volu, coef of opposite node is volu/tot_volu */
  
    /* volu = 1/6 * a x b * c */
    v_result( &node[tet[emas[e]].n[0]].nx, &node[tet[emas[e]].n[1]].nx, v12);   
    v_result( &node[tet[emas[e]].n[0]].nx, &node[tet[emas[e]].n[2]].nx, v13);   
    v_result( &node[tet[emas[e]].n[0]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[3]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[1]].nx, &node[tet[emas[e]].n[3]].nx, v12);   
    v_result( &node[tet[emas[e]].n[1]].nx, &node[tet[emas[e]].n[2]].nx, v13);   
    v_result( &node[tet[emas[e]].n[1]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[0]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[2]].nx, &node[tet[emas[e]].n[3]].nx, v12);   
    v_result( &node[tet[emas[e]].n[2]].nx, &node[tet[emas[e]].n[0]].nx, v13);   
    v_result( &node[tet[emas[e]].n[2]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[1]=v_sprod(vn,v14)/6./tet[emas[e]].v;
                                                    
    v_result( &node[tet[emas[e]].n[3]].nx, &node[tet[emas[e]].n[1]].nx, v12);   
    v_result( &node[tet[emas[e]].n[3]].nx, &node[tet[emas[e]].n[0]].nx, v13);   
    v_result( &node[tet[emas[e]].n[3]].nx, &node[nslav].nx, v14);   
    v_prod(v12,v13,vn);
    cof[2]=v_sprod(vn,v14)/6./tet[emas[e]].v;

    if(printFlag) printf(" e:%d cof:%e %e %e %e\n", emas[e]+1, cof[0], cof[1], cof[2], cof[3]);

    if(secondLoopFlag) break;
  
    /* a negativ cof means the nslav is outside, try the next element */
    secondLoopFlag=1;
    sum=0.;
    for (i=0; i<4; i++)
    {
      if (cof[i]<0.)
      {
        sum+=cof[i];
        secondLoopFlag=0;
      }
    }
    if((sum<0.)&&(sum>closestDist)) { closestElem=e; closestDist=sum; }
    if(secondLoopFlag) break;
  }
  if(!secondLoopFlag)
  {
    /* take the element with the smallest negative cof */
    e=closestElem;
    secondLoopFlag=1;
    goto secondLoop;
  }
  if(e==n_closest_tets) return(-1); 

  /* a negativ cof means the nslav is outside, set the cof to 0 and scale the rest to a sum of 1 */
  sum=0.;
  for (i=0; i<4; i++)
  {
    if (cof[i]<-1.e-2)
    {
      if(!extrapolflag) return(-1);
      cof[i]=0.;
    }
    else sum+=cof[i];
  }
  if(sum>0) for (i=0; i<4; i++) cof[i]/=sum;
  else return(-1);
  return(emas[e]);
}


/* return 0: ok, -1: no datasets, else setNr with nodes which could not be mapped */
int interpol3d(int set1, int set2, char *format, char *dataset, int extrapolflag)
{
#define MAX_NR_OF_TETS_PER_ELEM 44
#define N_CLOSEST_TETS 200
  int i,ii,j,n,e;
  int ds=0, allds=0;
  int etet, tets=0, n1,n2, elnr, failedNodesSet, coarseFlag;
  double volu=0, cg[3]={0,0,0}, cof[4], dr, dx, dy, dz,tol;
  static Elements  *elem=NULL;   
  static Nodes     *nod=NULL;
  static Tetraeder *tet=NULL;
  static Rsort *rsort=NULL;

  static double *orig_x=NULL, *orig_y=NULL, *orig_z=NULL, *sort_x=NULL, *sort_y=NULL, *sort_z=NULL;
  static int *sort_nx=NULL, *sort_ny=NULL, *sort_nz=NULL, near_node[N_CLOSEST_TETS];
  int     n_closest_nodes;

  static double *orig_ex=NULL, *orig_ey=NULL, *orig_ez=NULL, *sort_ex=NULL, *sort_ey=NULL, *sort_ez=NULL;
  static int *sort_enx=NULL, *sort_eny=NULL, *sort_enz=NULL;
  int     n_closest_tets;

  static Elements  elem2;   
  static Tetraeder *tet2=NULL;
  static Tetraeder *tetbuf=0;
  static int tetList[MAX_NR_OF_TETS_PER_ELEM];

  FILE *handle;

  printf("interpol3d\n");
  tol=gtol;
  tol*=tol;

  if ( (nod = (Nodes *)realloc( (Nodes *)nod, (anz->nmax+1) * sizeof(Nodes))) == NULL )
  {
    printf("WARNING: realloc error interpol3d\n");
  }
  if((elem = (Elements *)realloc( (Elements *)elem, (set[set2].anz_e) * sizeof(Elements))) == NULL )
  {
    printf("WARNING: realloc error interpol3d\n");
  }
  for (j=1; j<=anz->nmax; j++)
  {
    nod[j].nx=node[j].nx;
    nod[j].ny=node[j].ny;
    nod[j].nz=node[j].nz;
  }
  for (i=0; i<set[set2].anz_e; i++)
  {
    e=set[set2].elem[i];
    if(e_enqire[e].type==4) elem[i].type=1;
    else if(e_enqire[e].type==6) elem[i].type=3;
    else elem[i].type=e_enqire[e].type;
    elem[i].nr=e;
    for (j=0; j<26; j++) elem[i].nod[j]=e_enqire[e].nod[j];
  }

  if(dataset[0]=='d')
  {
    if(compareStrings( dataset, "ds" )>0)
    {
      /* all datasets should be interpolated */
      for(ds=0; ds<anz->l; ds++)
      {
        for(e=0; e<lcase[ds].ncomps; e++)
          printf(" interpol:%s entity:%s\n", lcase[ds].name, lcase[ds].compName[e]);
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[ds].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , ds, anz, node, lcase )==-1) 
          {
            printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", ds+1); 
    	    return(-1);
          }
          calcDatasets( ds, anz, node, lcase );
          recompileEntitiesInMenu(ds);
        }
      }
      allds=1;
    }
    else
    {
      ds=atoi(&dataset[2])-1; 
      if((ds<0)||(ds>anz->l-1)) { printf(" specified Dataset:%d not available\n",ds); return(-1); }
      for(e=0; e<lcase[ds].ncomps; e++)
        printf(" interpol:%s entity:%s\n", lcase[ds].name, lcase[ds].compName[e]);
      /* check if the data of the specified lcase (Dataset) are already available */
      if (!lcase[ds].loaded)
      {
        if( pre_readfrdblock(copiedNodeSets , ds, anz, node, lcase )==-1) 
        {
          printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", ds+1); 
          return(-1);
        }
        calcDatasets( ds, anz, node, lcase );
        recompileEntitiesInMenu(ds);
      }
    }
  }
  else { printf(" Dataset not given for interpolation of data\n"); return(-1); }


  /* split elements into tet's (with cg and volu) */
  tets = splitElementsToTets(set[set2].anz_e, nod, elem, &tet);
  if(!tets) { printf(" No 3D master elements found\n"); return(set1); }

  /* delete unusable tets (small volume) */
  j=0;
  for(i=0; i<tets; i++)
  {
    if(tet[i].v<SMALL_TET_VOLUME) { if(printFlag) printf("Scip Tet:%d vol:%e < SMALL_TET_VOLUME\n", i, tet[i].v); continue; }
    if(j<i)
    {   
      for(n=0;n<4; n++) tet[j].n[n]=tet[i].n[n];
      for(n=0;n<3; n++) tet[j].cg[n]=tet[i].cg[n];
      tet[j].v=tet[i].v;
      tet[j].e=tet[i].e;
    }
    j++;
  }
  tets=j;
  if(printFlag) printf(" %d tets created\n", tets);

  n_closest_nodes=1;

  handle = fopen ( "tets.inp", "w");
  fprintf(handle,"*NODE, NSET=Nall\n");
  for (i=0; i<anz->n; i++)
  {
    j=node[i].nr;
    //if( node[j].pflag == 0)
      fprintf(handle," %d,%f,%f,%f\n", j,node[j].nx,node[j].ny,node[j].nz  );
  }
  fprintf(handle,"*NODE, NSET=CG\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%f,%f,%f\n", i+anz->nmax+1, tet[i].cg[0], tet[i].cg[1], tet[i].cg[2] );
    cg[0]+=tet[i].cg[0];
    cg[1]+=tet[i].cg[1];
    cg[2]+=tet[i].cg[2];
  }
  cg[0]/=tets;
  cg[1]/=tets;
  cg[2]/=tets;
  fprintf(handle,"*ELEMENTS, TYPE=C3D4\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%d,%d,%d,%d\n", i+1, tet[i].n[0], tet[i].n[1], tet[i].n[2], tet[i].n[3]);
  }
  fprintf(handle,"*TEMPERATURE\nNall,0.\n");
  for(i=0; i<tets; i++)
  {
    fprintf(handle," %d,%e\n", i+anz->nmax+1,  tet[i].v );
    volu+=tet[i].v;
  }
  //printf("v:%f cg:%f %f %f\n",volu*scale->w*scale->w*scale->w,cg[0]* scale->w+scale->x,cg[1]* scale->w+scale->y,cg[2]* scale->w+scale->z);
  fclose(handle);
  
  if(set[set2].anz_n>tets)
  {
    if ( (rsort = (Rsort *)malloc( (set[set2].anz_n+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }
  else
  {
    if ( (rsort = (Rsort *)malloc( (tets+1) * sizeof(Rsort))) == NULL )
      printf("ERROR: realloc failed: Rsort\n\n" ); 
  }

  /* get the close node */
  /* search the closest node */
  if ( (orig_x = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_y = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_z = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_x = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_y = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_z = (double *)malloc( (set[set2].anz_n+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nx = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ny = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_nz = (int *)malloc( (set[set2].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_x[i]=node[set[set2].node[i]].nx;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_x[i]=rsort[i].r;
    sort_nx[i]=rsort[i].i;
  }
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_y[i]=node[set[set2].node[i]].ny;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_y[i]=rsort[i].r;
    sort_ny[i]=rsort[i].i;
  }
  for(i=0; i<set[set2].anz_n; i++)
  {
    rsort[i].r=orig_z[i]=node[set[set2].node[i]].nz;
    rsort[i].i=i;
  }
  qsort( rsort, set[set2].anz_n, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<set[set2].anz_n; i++)
  {
    sort_z[i]=rsort[i].r;
    sort_nz[i]=rsort[i].i;
  }
  
  /* get the close tets */
  /* search the closest elements based on the closest cg */
  if ( (orig_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (orig_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ex = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ey = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_ez = (double *)malloc( (tets+1) * sizeof(double))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enx = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_eny = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  if ( (sort_enz = (int *)malloc( (tets+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed \n\n" ); 
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ex[i]=tet[i].cg[0];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ex[i]=rsort[i].r;
    sort_enx[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ey[i]=tet[i].cg[1];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ey[i]=rsort[i].r;
    sort_eny[i]=rsort[i].i;
  }
  for(i=0; i<tets; i++)
  {
    rsort[i].r=orig_ez[i]=tet[i].cg[2];
    rsort[i].i=i;
  }
  qsort( rsort, tets, sizeof(Rsort), (void *)compareRsort );
  for(i=0; i<tets; i++)
  {
    sort_ez[i]=rsort[i].r;
    sort_enz[i]=rsort[i].i;
  }

  /* extend the datasets by the midface nodes later used in the process */
  if(allds)
  {
    /* all datasets should be interpolated */
    for(ds=0; ds<anz->l; ds++)
    {
      for(e=0; e<lcase[ds].ncomps; e++)
      {
        if((lcase[ds].dat[e] = (float *)realloc(lcase[ds].dat[e], (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n");
        /* block from nodalDataset() */
        for ( j=0; j<set[set2].anz_e; j++ )
        {
          elnr=set[set2].elem[j];
          switch(e_enqire[elnr].type)
          {
            case 4:
            for (n=0; n<3; n++)  /* create new vals at nodes in center of areas */
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[20+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[4+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[13+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[16+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[23]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[7]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[11]]+lcase[ds].dat[e][e_enqire[elnr].nod[12]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[19]]+lcase[ds].dat[e][e_enqire[elnr].nod[15]]) ;
            for (n=0; n<2; n++)  
            {
              n1=n*4;
              n2=n*8;
              lcase[ds].dat[e][e_enqire[elnr].nod[24+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n1]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[2+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n1]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[9+n2]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[10+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[11+n2]]) ;
            }
            break;
      
      
            case 5:
            for (n=0; n<2; n++) 
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[15+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[6+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[10+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[17]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[2]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[11]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[18]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[1]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 7]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[ 6]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 0]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[19]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[4]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5]]+lcase[ds].dat[e][e_enqire[elnr].nod[3]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[12]]+lcase[ds].dat[e][e_enqire[elnr].nod[13]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 3]]) ;
            break;
      
      
            case 10:
            lcase[ds].dat[e][e_enqire[elnr].nod[8]] = -0.25* (
            lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[1]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]])+0.5*(
            lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[6]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[7]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]]);
            break;
          }
        }
	  /* end block */
      }
    }
  }
  else
  {
    for(e=0; e<lcase[ds].ncomps; e++)
    {
        if((lcase[ds].dat[e] = (float *)realloc(lcase[ds].dat[e], (anz->nmax+1) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n");
        /* block from nodalDataset() */
        for ( j=0; j<set[set2].anz_e; j++ )
        {
          elnr=set[set2].elem[j];
          switch(e_enqire[elnr].type)
          {
            case 4:
            for (n=0; n<3; n++)  /* create new vals at nodes in center of areas */
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[20+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[4+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[13+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[16+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[23]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[7]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[11]]+lcase[ds].dat[e][e_enqire[elnr].nod[12]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[19]]+lcase[ds].dat[e][e_enqire[elnr].nod[15]]) ;
            for (n=0; n<2; n++)  
            {
              n1=n*4;
              n2=n*8;
              lcase[ds].dat[e][e_enqire[elnr].nod[24+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n1]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[2+n1]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n1]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[8+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[9+n2]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[10+n2]]+lcase[ds].dat[e][e_enqire[elnr].nod[11+n2]]) ;
            }
            break;
      
      
            case 5:
            for (n=0; n<2; n++) 
            {
              lcase[ds].dat[e][e_enqire[elnr].nod[15+n]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[1+n]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[4+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[3+n]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[6+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[10+n]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[12+n]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9+n]]) ;
            }
              lcase[ds].dat[e][e_enqire[elnr].nod[17]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[2]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 9]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[11]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[18]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[1]]+lcase[ds].dat[e][e_enqire[elnr].nod[0]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[ 8]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 7]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[ 6]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 0]]) ;
              lcase[ds].dat[e][e_enqire[elnr].nod[19]] = -0.25* (
                lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[4]]    +
                lcase[ds].dat[e][e_enqire[elnr].nod[5]]+lcase[ds].dat[e][e_enqire[elnr].nod[3]] )  + 0.5*(
                lcase[ds].dat[e][e_enqire[elnr].nod[12]]+lcase[ds].dat[e][e_enqire[elnr].nod[13]]   +
                lcase[ds].dat[e][e_enqire[elnr].nod[14]]+lcase[ds].dat[e][e_enqire[elnr].nod[ 3]]) ;
            break;
      
      
            case 10:
            lcase[ds].dat[e][e_enqire[elnr].nod[8]] = -0.25* (
            lcase[ds].dat[e][e_enqire[elnr].nod[0]]+lcase[ds].dat[e][e_enqire[elnr].nod[1]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[3]]+lcase[ds].dat[e][e_enqire[elnr].nod[2]])+0.5*(
            lcase[ds].dat[e][e_enqire[elnr].nod[4]]+lcase[ds].dat[e][e_enqire[elnr].nod[6]] +
            lcase[ds].dat[e][e_enqire[elnr].nod[7]]+lcase[ds].dat[e][e_enqire[elnr].nod[5]]);
            break;
          }
        }
	  /* end block */
    }
  }

  printf(" start interpolation\n");
  /* create set for failed nodes */
  delSet("-notMapped");
  failedNodesSet=pre_seta("-notMapped","i",0);
  for (i=0; i<set[set1].anz_n; i++ )
  {
    /* first, search for a close node */
    near3d(orig_x,orig_y,orig_z,sort_x,sort_y,sort_z,sort_nx,sort_ny,sort_nz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny,
           node[set[set1].node[i]].nz, set[set2].anz_n, &near_node[0], n_closest_nodes);
    //for (j=0; j<n_closest_nodes; j++) printf("node:%d near node:%d \n",set[set1].node[i], set[set2].node[near_node[j]]);
    dx= node[set[set2].node[near_node[0]]].nx - node[set[set1].node[i]].nx;
    dy= node[set[set2].node[near_node[0]]].ny - node[set[set1].node[i]].ny;
    dz= node[set[set2].node[near_node[0]]].nz - node[set[set1].node[i]].nz;      
    dr=dx*dx + dy*dy + dz*dz;      
    if(dr<tol )
    {
      if(printFlag) printf("node:%d, found equal node:%d with dr:%lf\n", set[set1].node[i], set[set2].node[near_node[0]], sqrt(dr));

      /* interpolate */
      if(allds)
      {
        /* all datasets should be interpolated */
        for(ds=0; ds<anz->l; ds++)
        {
          for(e=0; e<lcase[ds].ncomps; e++)
          {
            lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
          }
        }
      }
      else
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          lcase[ds].dat[e][set[set1].node[i]]=lcase[ds].dat[e][ set[set2].node[near_node[0]] ];
          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][ set[set1].node[i] ]);
        }
      }
      continue;
    }

    if((int)N_CLOSEST_TETS/10<tets)  n_closest_tets= (int)N_CLOSEST_TETS/10; else n_closest_tets= tets;
    coarseFlag=1;
  secondLoop:;
    /* if no close node exists, search a close element */
    near3d(orig_ex,orig_ey,orig_ez,sort_ex,sort_ey,sort_ez,sort_enx,sort_eny,sort_enz, node[set[set1].node[i]].nx,node[set[set1].node[i]].ny, node[set[set1].node[i]].nz, tets, &near_node[0], n_closest_tets);
    //if(printFlag) for (j=0; j<n_closest_tets; j++) printf("node:%d nearest tets:%d \n",set[set1].node[i], near_node[j]+1);

    etet=calcCoefficientsTet(set[set1].node[i], &near_node[0], n_closest_tets, node, tet, cof, extrapolflag);
    if(etet==-1)
    {
      if(coarseFlag) { coarseFlag=0; n_closest_tets*=10; goto secondLoop; }
      printf(" WARNING: no interpolation possible for node:%d\n", set[set1].node[i]);
      seta(failedNodesSet,"n",set[set1].node[i]);
      continue;
    }
    if(printFlag) printf("node:%d near tet:%d \n",set[set1].node[i], etet+1);

    if((e_enqire[elem[tet[etet].e].nr].type==4)||(e_enqire[elem[tet[etet].e].nr].type==6))
    {
      //printf("create finer tets for elem:%d\n",elem[tet[etet].e].nr);
      e=elem[tet[etet].e].nr;
      elem2.type=e_enqire[e].type;
      elem2.nr=e;
      for (j=0; j<26; j++) elem2.nod[j]=e_enqire[e].nod[j];

      j = splitElementsToTets(1, nod, &elem2, &tet2);

      /* delete unusable tets (small volume) */
      e=0;
      for(ii=0; ii<j; ii++)
      {
        if(tet2[ii].v<SMALL_TET_VOLUME) { continue; }
        if(e<ii)
        {   
          for(n=0;n<4; n++) tet2[e].n[n]=tet2[ii].n[n];
          for(n=0;n<3; n++) tet2[e].cg[n]=tet2[ii].cg[n];
          tet2[e].v=tet2[ii].v;
          tet2[e].e=tet2[ii].e;
        }
        e++;
      }
      
      //printf(" %d finer tets created\n", e);
      for(j=0; j<e; j++) tetList[j]=j;
      etet=calcCoefficientsTet(set[set1].node[i], &tetList[0], e, node, tet2, cof, extrapolflag);
      //printf("closest tet:%d\n", etet);

      /* temporary switch the tet pointer to the tet2 pointer */
      tetbuf=tet;
      //printf("switch the tet pointer forth %d\n", tetbuf);
      tet=tet2;
    }
    else tetbuf=0;

    /* interpolate */
    if(allds)
    {
      /* all datasets should be interpolated */
      for(ds=0; ds<anz->l; ds++)
      {
        for(e=0; e<lcase[ds].ncomps; e++)
        {
          if(etet>=0)
          {
            //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
            //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]], cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
            lcase[ds].dat[e][set[set1].node[i]]=0.;
            for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

            //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
          }
        }
      }
    }
    else
    {
      for(e=0; e<lcase[ds].ncomps; e++)
      {
        if(etet>-1)
        {
          //printf("ds:%d e:%d etet:%d \n",ds+1,e+1, etet);
          //for(j=0; j<4; j++) printf("n:%d val*cof: %f %f = %f\n",tet[etet].n[j], lcase[ds].dat[e][tet[etet].n[j]],cof[j], lcase[ds].dat[e][tet[etet].n[j]]*cof[j]);
          lcase[ds].dat[e][set[set1].node[i]]=0.;
          for(j=0; j<4; j++) lcase[ds].dat[e][set[set1].node[i]]+=lcase[ds].dat[e][tet[etet].n[j]]*cof[j];

          //printf("n:%d value:%f\n", set[set1].node[i], lcase[ds].dat[e][set[set1].node[i]]);
        }
        
      }
    }
    if(tetbuf)
    {
      /* switch the tet pointer back */
      //printf("switch the tet pointer back %d\n", tetbuf);
      tet=tetbuf;
      tetbuf=0;
    }
  }

  free(nod);
  free(elem);
  free(tet);
  nod=NULL;
  elem=NULL;

  if(rsort) free(rsort);
  if(orig_x) free(orig_x);
  if(orig_y) free(orig_y);
  if(orig_z) free(orig_z);
  if(sort_x) free(sort_x);
  if(sort_y) free(sort_y);
  if(sort_z) free(sort_z);
  if(sort_nx) free(sort_nx);
  if(sort_ny) free(sort_ny);
  if(sort_nz) free(sort_nz);

  if(orig_ex) free(orig_ex);
  if(orig_ey) free(orig_ey);
  if(orig_ez) free(orig_ez);
  if(sort_ex) free(sort_ex);
  if(sort_ey) free(sort_ey);
  if(sort_ez) free(sort_ez);
  if(sort_enx) free(sort_enx);
  if(sort_eny) free(sort_eny);
  if(sort_enz) free(sort_enz);

  printf("done\n");
  return(failedNodesSet);
}



int brecord( char *rec_str, char dat[MAX_PARAM_IN_BRECORD][MAX_LINE_LENGTH])
{
  int i, length;
  int nextarg=0, letter=0;

  length=strlen(rec_str);
  /* scan all args divided by comma */
  nextarg=0;letter=0;
  for(i=0; i<=length; i++)
  {
    if(rec_str[i]==(char)EOF) {dat[nextarg][letter]='\0';  break; } 
    if(rec_str[i]=='\n') {dat[nextarg][letter]='\0';  break; } 
    if(rec_str[i]==0) {dat[nextarg][letter]='\0';  break; } 
    if((rec_str[i]==' ')&&(letter))
    {
      dat[nextarg][letter]='\0';
      nextarg++;
      letter=0;
      if(nextarg>=MAX_PARAM_IN_BRECORD) { printf("ERROR: too much parameters in line:\n%s\n\n", rec_str); exit(1); }
    }
    else
    {
      if((rec_str[i]>32)&& (rec_str[i]<127))
      {
        dat[nextarg][letter]=rec_str[i];
        letter++;
        if(nextarg>=MAX_LINE_LENGTH) { printf("ERROR: brecord, increase MAX_LINE_LENGTH in extUtil.h\n\n"); exit(1); }
      }
    }
  }
  if(dat[nextarg][0]=='\0') return(nextarg);
  return(nextarg+1);
}

/*------------------------------------------------------------------*/
/* Daten im fbd-format einlesen                                     */
/*------------------------------------------------------------------*/
/* return <0 if reading has to be stopped abnormally */
int commandoInterpreter( char *type, char *string, int na, int nb, FILE *handle1, int addFlag, int *gtolFlag )
{
  int   i,j,k,l,ibuf=0; 
  int nr;

  char  buffer[MAX_LINE_LENGTH];
  char  setname[MAX_LINE_LENGTH], setname2[MAX_LINE_LENGTH];
  char  xbuf[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH], dataset[MAX_LINE_LENGTH];
  char  name[MAX_LINE_LENGTH], comm[MAX_LINE_LENGTH], posn[MAX_LINE_LENGTH], entity[MAX_LINE_LENGTH];
  char  addDispFlagLocal=0;
  static char  **seq=NULL;
  char  **buf;
  char  dat[MAX_PARAM_IN_BRECORD][MAX_LINE_LENGTH];
  int       args,anz_seq, length, setNr, setNr2;
  double    pscal, x,y,dx,dy, wx[2], wy[2];
  char      **substring=NULL;
  int       foundSubString;
  static int *picnr=NULL;
  int pics;
  int pnts[3];
  DIR *dirp;
  struct direct *dp;
  static int ifFlag=1, whileFlag=1, returnFlag=1;
  char parameter[20][MAX_LINE_LENGTH];

  for(j=0;j<strlen(type); j++) type[j]=toupper(type[j]);
 whileLoop:;
  for(j=0;j<MAX_PARAM_PER_RECORD; j++) dat[j][0]=0;
  for(j=0;j<strlen(string); j++) if(string[j]=='\n') string[j]=0;

  /* check if a loop starts due to "WHILE" */ 
  /* this has to be evaluated before a substitution of values happened! */
  /* whileFlag=0 condition was not fulfilled, skip the command
     whileFlag=1 execute command
     whileFlag=2 store and execute command
     whileFlag=3 restore and execute command and at the end of commandoInterpreter() go back to whileLoop:;
     whileFlag=4 store command but do not execute
  */

  // save (whileFlag=2) or restore (3) command and execute later
  // save command but do not execute (4)
  if(whileFlag>1)
  {
    whileFlag=pre_while(type, string, &na, &nb, handle1, &addFlag, gtolFlag, whileFlag);
  }
  // generate data-structures
  if( (compareStrings(type, "WHILE")>0)||(compareStrings(type, "ENDWHILE")>0))
  {
    whileFlag=pre_while(type, string, &na, &nb, handle1, &addFlag, gtolFlag, 0);
    goto checkForError;
  }
  if((whileFlag==0)||(whileFlag==4)) goto checkForError;


  /* replace parameters by defined values */
  //if ((compare(type, "VALU", 4)!=4)&&(compare(type, "PRNT",4)!=4)&&(compare(type, "SET", 3)!=3))
  if (compare(type, "VALU", 4)!=4)
  {
    args=brecord(&string[na], dat);
    string[na]=0;
    for(i=0; i<args; i++)
    {
      j=getValuNr(dat[i]);
      if (j>-1)
      {
        //printf (" param:%s replaced by value:%s\n", value[j].name, value[j].string );
        strcpy(dat[i],value[j].string );
      }
      sprintf(&string[strlen(string)]," %s",dat[i]);
    }
    //printf("replaced parameters by defined values: %s\n", string);
  }
  /* substitution of values in PRNT leads to errors!  */

  /* check for "IF" statements */ 
  if((compareStrings(type, "IF")>0)||(compareStrings(type, "ELSE")>0)||(compareStrings(type, "ENDIF")>0))
  {
    ifFlag=pre_if(string, ifFlag);
    goto checkForError;
  }
  if(ifFlag==0) goto checkForError;

  setname[0]=name[0]=posn[0]=comm[0]=entity[0]=xbuf[0]=format[0]=0;
  //printf("string to eval:%s\n", string);
  //printf("string[na+1]:%s\n", &string[na+1]);

  /* remove masking of parameters (usually values) */
  if(anz->v) for(i=0; i<strlen(string); i++) if(string[i]=='\\') string[i]=' ';

  if (compareStrings(type, "ANIM")>0)
  {
    pre_animate(&string[na+1]);
  }
  else if (compareStrings(type, "AREA")>0)
  {
    pre_area(&string[na+1]);
  }
  else if (compareStrings(type, "ASGN")>0)
  {
    sscanf(&string[na+1], "%s %s", entity, name);
    if(( entity[0]=='s')&&( entity[1]=='e')) lchar->se=name[0];
    else if( compareStrings(entity, "alpha")>0) meshp.alpha=atof(name);
    else if( compareStrings(entity, "beta")>0) meshp.beta=atof(name);
    else if( compareStrings(entity, "mpc")>0) nasMpc=1;
    else if( compareStrings(entity, "nadapt")>0) meshp.nadapt=atoi(name);
    else if( compareStrings(entity, "rbe")>0) { nasMpc=0; nasRbeHec=atof(name); }
    else if( entity[0]=='n') anz->noffs=atoi(name)-1;
    else if( entity[0]=='e') anz->eoffs=atoi(name)-1;
    else if( entity[0]=='p') lchar->p=name[0];
    else if( entity[0]=='l') lchar->l=name[0];
    else if( entity[0]=='c') lchar->c=name[0];
    else if( entity[0]=='s') lchar->s=name[0];
    else if( entity[0]=='b') lchar->b=name[0];
    else if( entity[0]=='L') lchar->L=name[0];
    else if( entity[0]=='S') lchar->S=name[0];
    else if( entity[0]=='h') lchar->sh=name[0];
    else errMsg(" %s not recognized\n", entity);
    printf("rbe %d %f %8f %s\n", nasMpc, nasRbeHec, nasRbeHec, name);
  }
  else if (compareStrings(type, "BIA")>0)
  {
    pre_bia(&string[na+1]);
  }
  else if (compareStrings(type, "BODY")>0)
  {
    pre_body(&string[na+1]);
  }
  else if (compareStrings(type, "BREAK")>0)
  {
    printf("BREAK by user request\n");
    return(-1);
  }
  else if (compareStrings(type, "CALL")>0)
  {
    /* when node coordinates were changed to the deformed ones then switch back */ 
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      // remember to switch back
      addDispFlagLocal=2;
    }
    if(vectorFlag) pre_view("vector off");
    descalAll();
    userFunction(&string[na+1], anz, anzGeo);
    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalPoints ( anzGeo->p, point, scale );
    scalNodes ( anz->n, node, scale);
    scalSurfs( anzGeo->s, surf, scale);

    createDatasetEntries();
    if(anz->e>0)
    {
      adjustDrawNodes(0);
      makeSurfaces();
      getElemNormalen( e_enqire, node, anz->e );
      updateDispLists();
    }
    /* when node coordinates were changed to the deformed ones then switch back before they are copied and then switch again */ 
    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
  }
  else if (compareStrings(type, "CNTR")>0)
  {
    if ( sscanf(&string[na+1], "%s", dat[0]) < 1) 
    { printf(" ERROR in formulation\n"); goto checkForError; }

    i=0;
    nr=getPntNr(dat[i]);
    if(nr<0)
    {
      if(checkIfNumber(dat[i])==0)
      {
        printf(" ERROR: arg:%s is no point and no node\n", dat[i]);
        goto checkForError;
      }
      nr=atoi(dat[i]);
      center( node[nr].nx, node[nr].ny, node[nr].nz);
    }
    else  center( point[nr].px, point[nr].py, point[nr].pz);
  } 
  else if (compareStrings(type, "COPY")>0)
  {
    pre_copy(&string[na+1]);
    redraw();
  }
  else if (compareStrings(type, "COMP")>0)
  {
    if ( sscanf(&string[na+1], "%s%s", name, xbuf) < 2) 
    { printf(" ERROR in formulation\n"); goto checkForError; }

    setNr=getSetNr(name);
    if (setNr<0)
    {
      /* set was not found. check if wildcards (*) were used */
      if((strlen(name)==1)&&(name[0]=='*'))
      {
        for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
        {
          if (!set[setNr].type)
          {
            printf(" comp %s %s\n", set[setNr].name, xbuf) ;
            completeSet( set[setNr].name, xbuf) ;
          }
        }
        goto checkForError;
      }
      length= strsplt( name, '*', &buf);
      if (length>0)
      {
        j=0;
        for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
        {
          foundSubString=0;
          for(i=0; i<length; i++)
          {
            if(strstr(set[setNr].name, buf[i]) !=NULL)
            {
              foundSubString++;
              /* check if the first or the last char is no '*' then the buf[] must be at start or end */
              if(i==0) { if(name[0]!='*')  { if(name[0]!=set[setNr].name[0])  foundSubString--; }  }
              if(i==length-1) { if(name[strlen(name)-1]!='*') { if(name[strlen(name)-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
	    }
	  }
          if(foundSubString==length)
	  {
            i=setNr;
            if (!set[i].type)
            {
              j++;
              printf(" comp %s %s\n", set[i].name, xbuf) ;
              completeSet( set[i].name, xbuf) ;
            }
	  }
	}
        if(j!=0) goto checkForError;
      }
      /* free buf */
      for(i=0; i<length; i++) free(buf[i]);
      free(buf);

      printf (" WARNING(comp): set '%s' does not exist\n", name);
      goto checkForError;
    }
    else  completeSet( name, xbuf) ;
  }
  else if (compareStrings(type, "CORRAD")>0)
  {
    length=sscanf(string, "%*s %s %lf",name,&x); 
    if (length==1) x=0.;
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" corrad: set:%s does not exist\n", name);
      goto checkForError;
    }
    corrad( setNr, x);
  }
  else if (compareStrings(type, "CSYSA")>0)
  {
    length = sscanf(&string[na+1], "%d %s", &nr, name);
    if(length<2) { printf(" wrong syntax\n"); goto checkForError; }
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" ERROR in csysa: set:%s does not exist\n", name);
      goto checkForError;
    }
    nodeCsysSet[nodeCsys]=setNr;
    nodeCsysNr[nodeCsys]=nr;
    nodeCsys++;
  }
  else if (compareStrings(type, "CUT")>0)
  {
    ibuf=sscanf(&string[na+1], "%s %s %s", dat[0], dat[1], dat[2]);
    if(ibuf==1)
    {
      nr=atoi(dat[0]);
      if(nr<1)
      {
        printf(" ERROR: arg:%s is no node\n", dat[i]);
        goto checkForError;
      }
      pre_cut( nr, 'v' );
    }
    else if(ibuf==3)
    {
     for (i=0; i<3; i++)
     {
      nr=getPntNr(dat[i]);
      if(nr<0)
      {
        if(checkIfNumber(dat[i])==0)
	{
          printf(" ERROR: arg:%s is no point and no node\n", dat[i]);
          goto checkForError;
        }
        nr=atoi(dat[i]);
        pre_cut( nr, 'n' );
      }
      else pre_cut( nr, 'p' );
     }
    }
    else
    {
      printf(" ERROR: wrong number of arguments. One node or 3 points or nodes needed\n");
      goto checkForError;
    }
    goto checkForError;
  }
  else if (compareStrings(type, "DEL")>0)
  {
    pre_del(&string[na+1]);
  }
  else if (compareStrings(type, "DIST")>0)
  {
    pre_proj( string );
  }
  else if (compareStrings(type, "DIV")>0)
  {
    pre_div(&string[na+1]);
  }
  else if ((compareStrings(type, "DS")>0)||(compareStrings(type, "LC")>0))
  {
    selectData( &string[na+1] );
  }
  else if (compareStrings(type, "ELEM")>0)
  {
    pre_elem(&string[na+1]);
  }
  else if (compareStrings(type, "ELTY")>0)
  {
    pre_elty(&string[na+1]);
  }
  else if (compareStrings(type, "ENQ")>0)
  {
    enquireEntities(&string[na+1]);
  }
  else if (compareStrings(type, "EQAL")>0)
  {
    pre_eqal(&string[na+1]);
  }
  else if ( compareStrings(type, "EXIT")>0 )
  {
    strcpy(setname,datin);
    for (i=strlen(datin); i>=0; i--)
    {
      if(datin[i]=='.')break;
    }
    setname[i]='\0';
#ifdef WIN32
    sprintf (buffer, "move /y \"%s\" \"%s.fbb\"", datin, setname );
#else
    sprintf (buffer, "mv %s %s.fbb", datin, setname );
#endif
    if(compare(&datin[i+1],"fbd",3)==3) system (buffer);
    pre_write(" all fbd ");
    if( compareStrings( setname, "all")<3)
    {
#ifdef WIN32
      sprintf (buffer, "move /y all.fbd \"%s.fbd\"", setname );
#else
      sprintf (buffer, "mv all.fbd %s.fbd", setname );
#endif
      system (buffer);
    }     
    exit(0);
  }
  else if (compareStrings(type, "FLIP")>0)
  {
    na=sscanf(string, "%*s%s%s",dat[0],dat[1]);
    nr=getSetNr(dat[0]);
    if((nr>-1)&&(set[nr].type==1)) nr=-1;
    if(nr>-1)
    {
      for(i=0; i<set[nr].anz_s; i++) flip("s",set[nr].surf[i]);
      for(i=0; i<set[nr].anz_b; i++) flip("b",set[nr].body[i]);
    }
    else
    {
      if((getSurfNr(dat[0])>-1)&&(getBodyNr(dat[0])>-1)) { printf(" ERROR Name points to body and surface %s\n", dat[0]); goto checkForError ; }
      nr=getSurfNr(dat[0]);
      if(nr>-1)
      {
        if((na>1)&&(dat[1][0]=='a'))  flip("sa",nr);
        else flip("s",nr);  
      }
      else
      {
        nr=getBodyNr(dat[0]);
        if(nr>-1)
        {
          flip("b",nr);  
        }
        else printf(" ERROR Name unknown:%s\n",dat[0]);
      }
    }
  }
  else if (compareStrings(type, "FLPC")>0)
  {
    flipColorFlag=!flipColorFlag;
    redraw();
  }
  else if (compareStrings(type, "FONT")>0)
  {
    i=DEF_GLUT_FONT+1;
    sscanf(&string[na+1],"%s %d",buffer,&i);
    if((i>0)&&(i<=SUM_GLUT_FONTS))
    {
      i--;
      if(compare(buffer, "legend", 1)==1) legend_font=i;
      else if(compare(buffer, "draw", 1)==1) draw_font=i;
      else if(compare(buffer, "menu", 1)==1) menu_font=i;
      else printf(" WARNING: Unknown target:%s\n", buffer);
      redraw();
    }
    else printf(" WARNING: Unknown font index:%d\n",i);
  }
  else if (compareStrings(type, "FRAME")>0)
  {
    frame();
  }
  else if (compareStrings(type, "GBOD")>0)
  {
    pre_gbod(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "GLUE")>0)
  {
    pre_contact(&string[na+1]);
  }
  else if (compareStrings(type, "GONLY")>0)
  {
    if (( compareStrings(&string[na+1], "of")>0)
        ||( compareStrings(&string[na+1], "OF")>0) )
    { 
      frameFlag=1;
      filenamFlag=1;
      textFlag=1;
      scalaFlag=1; 
    }
    else
    { 
      frameFlag=0;
      filenamFlag=0;
      textFlag=0;
      scalaFlag=0;
    }
    redraw();
    glutSetWindow( w0);
    glutPostRedisplay();
  }
  else if (compareStrings(type, "GRAPH")>0)
  {
    graph(&string[na+1]);
  }
  else if (compareStrings(type, "GRPA")>0)
  {
    printf("ERROR: Not longer supported.\n");
  }
  else if (compareStrings(type, "GROUPS")>0)
  {
    printf("ERROR: Not longer supported.\n");
  }
  else if (compareStrings(type, "GRPS")>0)
  {
    for (i=0; i<anz->e; i++)
    {
      sprintf(name,"%d", e_enqire[i].nr);
      sprintf(buffer, "+grp%d", e_enqire[e_enqire[i].nr].group );
      pre_seta( buffer, "e", name  );
    }
    prnt("se");
  }
  else if (compareStrings(type, "GROUP")>0)
  {
    printf("ERROR: Not longer supported.\n");
  }
  else if (compareStrings(type, "GSUR")>0)
  {
      pre_gsur(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "GTOL")>0)
  {
    if(gtolFlag>(int *)NULL) *gtolFlag=1;
    length=sscanf(string, "%*s %lf",&gtol); 
    if (length==1) printf ("gtol set to:%e\n", gtol);
    else { gtol=calcGTOL(setall);  printf ("gtol new calculated:%e\n", gtol); }
  }
  else if (compareStrings(type, "HCPY")>0)
  {
    format[0]=0;
    sscanf(string,"%*s %s %s", name, format);
    if     (compareStrings(name, "ps")>0)  createHardcopy(1);
    else if(compareStrings(name, "tga")>0) createHardcopy(2);
    else if(compareStrings(name, "gif")>0) createHardcopy(4);
    else if(compareStrings(name, "png")>0) createHardcopy(5);
    else if(compareStrings(name, "clean")>0)
    {
      sprintf( buffer, "rm -f hcpy_*.ps %s", DEV_NULL2);
      system (buffer);
      psNr=1;
    }
    else if(compareStrings(name, "make")>0)
    {
      /* open the actual dir and get the available hcpy files */
      pics=0;
      dirp = opendir(".");
      if (dirp != NULL) while ((dp = readdir(dirp)) != NULL)
      {
        /* search for hcpy_xx.ps files  */
        if( (compare(dp->d_name, "hcpy", 4) == 4) && ( dp->d_name[strlen(dp->d_name)-2]=='p') && ( dp->d_name[strlen(dp->d_name)-1]=='s') )
        {
          printf(" found %s \n", dp->d_name);
          /* get the pic-nr */
          i=0; while(dp->d_name[i]!='_') i++; i++;
          j=0; while(dp->d_name[i]!='.') { posn[j]=dp->d_name[i]; j++; i++;}
          posn[j]=0;
          if ( (picnr = (int *)realloc( (int *)picnr, (pics+2) * sizeof(int))) == NULL )
          {  printf("\n\n ERROR: realloc failed, hcpy\n\n"); }
          picnr[pics]=atoi(posn); pics++;
        }
      }
      closedir(dirp);
      qsort( picnr, pics, sizeof(int), (void *)compareInt );

      nr=0; /* page-nr */
      ibuf=0; /* pic-nr */
      do
      {
        sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=tmp.ps ");
        for(j=0; j<6; j++) /* 6 pics per page */
	{
          if(ibuf==pics) break;
          sprintf( &buffer[strlen(buffer)], " hcpy_%d.ps", picnr[ibuf++]);
	}
        system (buffer);
        printf("%s\n",buffer);
        if ( compareStrings(format, "ls")>0)
        {
          pscal=0.42;
          x=11.; dx=9.;
          y=1.5;  dy=9.;
          /* sscanf(string,"%*s %*s %*s %lf %lf %lf %lf %lf", &x, &y, &dx, &dy, &pscal); */
          
          // activate this lined if ps2write is used
          sprintf( buffer, "convert -density %dx%d tmp.ps tmp.ps", (int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)),(int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)));
          system (buffer);
          printf("%s\n",buffer);

          sprintf( buffer, "pstops '6:0L@%lf(%lfcm,%lfcm)+3L@%lf(%lfcm,%lfcm)+1L@%lf(%lfcm,%lfcm)+4L@%lf(%lfcm,%lfcm)+2L@%lf(%lfcm,%lfcm)+5L@%lf(%lfcm,%lfcm)' tmp.ps tmp_%d.ps", pscal, x,y, pscal, x+dx,y, pscal, x,y+dy, pscal, x+dx,y+dy, pscal, x,y+2.*dy, pscal, x+dx,y+2.*dy, nr);
        }
        else
        {
          pscal=0.42;
          x=2.; dx=9.;
          y=1.; dy=9.;
          /* sscanf(string,"%*s %*s %*s %lf %lf %lf %lf %lf", &x, &y, &dx, &dy, &pscal); */
          
          // activate this lined if ps2write is used
          sprintf( buffer, "convert -density %dx%d tmp.ps tmp.ps", (int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)),(int)((double)(PS_DENSITY*width_w0)/(double)(INI_SCREEN+INI_MENU_WIDTH)));
          system (buffer);
          printf("%s\n",buffer);

          sprintf( buffer, "pstops '6:4@%lf(%lfcm,%lfcm)+5@%lf(%lfcm,%lfcm)+2@%lf(%lfcm,%lfcm)+3@%lf(%lfcm,%lfcm)+0@%lf(%lfcm,%lfcm)+1@%lf(%lfcm,%lfcm)' tmp.ps tmp_%d.ps", pscal, x,y, pscal, x+dx,y, pscal, x,y+dy, pscal, x+dx,y+dy, pscal, x,y+2.*dy, pscal, x+dx,y+2.*dy, nr);
        }
        system (buffer);
        printf("%s\n",buffer);
        sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=cgx_%d.ps tmp_%d.ps", nr, nr);
        system (buffer);
        printf("%s\n",buffer);
        sprintf( buffer, "rm -f tmp*.ps %s",DEV_NULL);
        system (buffer);
        nr++;
      }while(j==6);
      sprintf( buffer, "gs -dNOPAUSE -dBATCH -sDEVICE=ps2write -sOutputFile=cgx.ps ");
      for(j=0; j<nr; j++)
      {
        sprintf( &buffer[strlen(buffer)], " cgx_%d.ps", j);
      }
      system (buffer);
      printf("%s\n",buffer);
      sprintf( buffer, "rm -f cgx_*.ps %s",DEV_NULL);
      system (buffer);
      // TBD: if ( compareStrings(format, "ls")>0)
      if(inpformat) sprintf(buffer, "%s cgx.ps &", psviewer);
      system (buffer);
      printf("ready\n");

      // activate this lined if ps2write is used
      printf(" you may convert to pdf with 'convert -density 600x600 cgx.ps cgx.pdf\n");

    }
    else if(compareStrings(&string[na+1], "clean")>0)
    {
      sprintf( buffer, "rm -f hcpy_* %s",DEV_NULL);
      system (buffer);
      psNr=tgaNr=gifNr=pngNr=1;
    }           
    else createHardcopy(2);
  }
  else if (compareStrings(type, "LCMB")>0)
  {
    pre_lcmb(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "LENGTH")>0)
  {
    pre_length(&string[na+1]);
  }
  else if (compareStrings(type, "LINE")>0)
  {
    pre_line(&string[na+1], addFlag);
  }
  else if (compareStrings(type, "LNOR")>0)
  {
    if (!addFlag)    length = sscanf(string, "%*s%s%s%s%s%lf", name, dat[1], dat[2], dat[0], &pscal);
    else 
    {
      name[0]='!';
      dat[0][0]=dat[1][0]=dat[2][0]='%';
      length = sscanf(string, "%*s%s%s%s%s%lf", &name[1],  &dat[1][1],  &dat[2][1],  &dat[0][1], &pscal);
    }
    operateAlias( name, "l" );
    operateAlias( dat[0], "p" );
    operateAlias( dat[1], "p" );
    operateAlias( dat[2], "p" );
  
    /* check if a lcmb has this name, not use it */
    if (getLcmbNr(name) >0) {errMsg("ERROR: LCMB uses this name %s, try another name \n", name); return(-1);}
    if (!pscal) {errMsg("ERROR: length must not be 0\n"); return(-1);}

    for(i=0; i<3; i++) pnts[i]=getPntNr(dat[i]);
    if((i=normalLine(name, pnts, pscal))<0)
    { printf("lnor: could not create new line\n");  return(-1);}
    else { printf(" create line:%s with new end-point %s\n", name, point[i].name ); }
  }
  else if (compareStrings(type, "MAP")>0)
  {
    dat[0][0]=0;
    length = sscanf(string, "%*s%s%s%s%s%s", setname, setname2, name, dataset, dat[0]);
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" set:%s does not exist\n", setname);
      goto checkForError;
    }
    setNr2=getSetNr(setname2);
    if (setNr2<0)
    {
      printf (" set:%s does not exist\n", setname2);
      goto checkForError;
    }
    if (set[setNr2].anz_n<=0)
    {
      printf (" set:%s does not contain nodes\n", setname2);
      goto checkForError;
    }

    if ((compare(name, "surf", 1)==1)||(name[0]== 'r')||(name[0]== 'x')||(name[0]== 'y')||(name[0]== 'z'))
    {
      if(dat[0][0]=='n') areampc(setNr, setNr2, "map", name, dataset, 0, 0, node, 0);
      else areampc(setNr, setNr2, "map", name, dataset, 0, 0, node, 1);
    }
    if (compare(name, "volu", 1)==1)
    {
      if(dat[0][0]=='n') interpol3d(setNr, setNr2, name, dataset, 0);
      else interpol3d(setNr, setNr2, name, dataset, 1 );
    }
    redraw();
  }
  else if (compareStrings(type, "MATA")>0)
  {
    length = sscanf(&string[na+1], "%d %s", &nr, name);
    if(length<2) { printf(" wrong syntax\n"); goto checkForError; }
    operateAlias( name, "se" );
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" ERROR in mata: set:%s does not exist\n", name);
      goto checkForError;
    }
    mata( elemMat, nr, setNr);
  }
  else if (compareStrings(type, "MATS")>0)
  {
    for (i=1; i<=elemMat[0]; i++) printf ("matNr:%d\n", elemMat[i]); 
    for (i=0; i<anz->e; i++)
    {
      sprintf(name,"%d", e_enqire[i].nr);
      sprintf(buffer, "+mat%d", e_enqire[e_enqire[i].nr].mat );
      pre_seta( buffer, "e", name );
    }
  }
  else if (compareStrings(type, "MAX")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); return(1); }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    redraw();
  }
  else if (compareStrings(type, "MERG")>0)
  {
    pre_merge( &string[na+1] );
    redraw();
  }
  else if (compareStrings(type, "MESH")>0)
  {
    if(addDispFlag)
    {
      addDispToCoordinates(node);
      addDispFlagLocal=2;
    }
    pre_mesh( &string[na+1] );
      adjustDrawNodes(0);
      getElemNormalen( e_enqire, node, anz->e );
      makeSurfaces();
    if(addDispFlagLocal==2)
    {
      addDispToCoordinates(node);
    }
    redraw();
  }
  else if (compareStrings(type, "MIDS")>0)
  {
    if(addDispFlag==1)
    {
      printf (" WARNING: displacements were added to the node-coords. This is not permitted\n");
      addDispToCoordinates(node);
    }
    pre_view(" surf");
    sscanf( string, "%*s %s %s", setname, comm);
    fixMidsideNodes( setname, comm );
    makeSurfaces();        // includes getFaceNormalen 
    adjustDrawNodes(0);
    getElemNormalen( e_enqire, node, anz->e );
    updateDispLists();
    printf (" WARNING: Results were intentionally not interpolated to the new nodes. Do not work with Datasets any more.\n");
    redraw();
  }
  else if (compareStrings(type, "MINUS")>0)
  {
    minus(&string[na+1] );
  }
  else if (compareStrings(type, "MIN")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); return(1); }
    sscanf( string, "%*s %lf %c %c",&scale->smin,&scale->format,&scale->lock );
    redraw();
  }
  else if (compareStrings(type, "MM")>0)
  {
    if((animFlag)&&(!halfperiod)) { printf("ERROR: The scale can not be changed during animation\n\n"); return(1); }
    sscanf( string, "%*s %lf %c %c",&scale->smax,&scale->format,&scale->lock );
    scale->smin=-scale->smax;
    redraw();
  }
  else if (compareStrings(type, "MOVE")>0)
  {
    pre_move( &string[na+1] );
    redraw();
  }
  else if (compareStrings(type, "MOVI")>0)
  {
    pre_movie( &string[na+1] );
  }
  else if ( compareStrings(type, "MSG")>0 )
  {
    sscanf( string, "%*s%s", buffer );
    if ( compareStrings(buffer, "on")>0) printFlag=1; 
    else if ( compareStrings(buffer, "off")>0) printFlag=0;
    else printf(" %s not recognized (use on/off)\n", buffer);
  }
  else if (compareStrings(type, "NODE")>0)
  {
    if(addDispFlag==1) { printf (" WARNING: displacements were added to the node-coords. This is not permitted\n"); addDispToCoordinates(node); }
    pre_nod(&string[na+1]);
    getElemNormalen( e_enqire, node, anz->e );
    makeSurfaces();
    realloc_colNr();
    updateDispLists();
  }
  else if (compareStrings(type, "NORM")>0)
  {
    pre_norm(&string[na+1]);
  }
  else if (compareStrings(type, "NURL")>0)
  {
    nurl( &string[na+1], addFlag );
  }
  else if (compareStrings(type, "NURS")>0)
  {
    length = sscanf( string, "%*s%s%s%s", name, comm, buffer  );

    /* check if a set of blended surfaces should make nurbs. The surfaces use then this nurbs */ 
    if((length==2) && ( name[0] == '!' ))
    {
      setNr=getSetNr(comm);
      if(setNr<0)
      {
        printf("ERROR: Set:[%s] in command:|%s| does not exist\n",comm,string);
        goto checkForError;
      }
      /* try to create suitable nurbs */
      ibuf=set[setNr].anz_s;
      for ( i=0; i<ibuf; i++)
      {
        nr = set[setNr].surf[i];
        if (surf[nr].sh==-1)
        {
          k=createBlendedNurbs(nr, &point, line, lcmb, surf, &nurbs );
          if(k>-1)
	  {
            seta( getSetNr("all"), "S", k );
            for (j=0; j<anz->sets; j++)
            {
              if(( set[j].name != (char *)NULL)&&( set[j].flag=='o')) seta( j, "S", k );
            }

            /* create a shape of the same name for reference in surfaces */
            surf[nr].sh=shape_i( nurbs[k].name, 4, k, 0, 0, 0, 0, 0, 0);
            repNurs(k);
            repSurf(nr,0);
	  }
	}
      }
    }
    else nurs( &string[na+1], addFlag);
  }
  else if (compareStrings(type, "ORI")>0)
  {
    orientSet( &string[na+1] );
  }
  else if (compareStrings(type, "PLOT")>0)
  {
    /* trigger new scale-values */
    length=sscanf(&string[na+1],"%s", format);
    if((compareStrings(format,"ev")>0)||(compareStrings(format,"fv")>0)) { if(scale->lock!='l') scale->smin=scale->smax=0; }
    plot( &string[na+1] );
  }
  else if (compareStrings(type, "PLUS")>0)
  {
    /* trigger new scale-values */
    if((compareStrings(format,"ev")>0)||(compareStrings(format,"fv")>0)) { if(scale->lock!='l') scale->smin=scale->smax=0; }
    plus( &string[na+1] );
  }
  else if (compareStrings(type, "PNT")>0)
  {
    pre_pnt( &string[na+1], addFlag );
  }
  else if (compareStrings(type, "PROJ")>0)
  {
    if(addDispFlag==1) { printf (" WARNING: displacements were added to the node-coords. Projection of nodes might fail.\n"); }
    pre_proj( string );
  }
  else if (compareStrings(type, "PROC")>0)
  {
    pre_proc( &string[na+1] );
  }
  else if (compareStrings(type, "PRNT")>0) prnt(&string[na+1]);
  else if (compareStrings(type, "QUIT")>0) exit(0);
  else if (compareStrings(type, "READ")>0)
  {
    pre_read(&string[na+1]); 
  }
  else if (compareStrings(type, "REP")>0)
  {
    nr=getSetNr(&string[na+1]);
    if(nr<0)
    {
      printf("ERROR: Set:[%s] in command:|%s| does not exist\n",&string[na+1],string);
      goto checkForError;
    } 
    //orientSet( &string[na+1] );
    delSet(specialset->bsur);
    if( compareStrings(&string[na+1],"all")>0)
    {
      for (i=0; i<anzGeo->l; i++) repLine(i);
      for (i=0; i<anzGeo->nurl; i++) repNurl(i);
      for (i=0; i<anzGeo->nurs; i++) repNurs(i);
      /* delete the data-structures for trimming of the nurbs */
      for (i=0; i<anzGeo->nurs; i++) untrimNurs(i);
      /* recalculate the nurbs-trimming-points (after repNurs) */
      /* set "all" to avoid substitute surfs */
      j=getSetNr("all");
      if(j>=0) for (i=0; i<set[j].anz_s; i++) repSurf(set[j].surf[i],1);
    }
    else
    {
      for (i=0; i<set[nr].anz_l; i++) repLine(set[nr].line[i]);
      for (i=0; i<set[nr].anz_nurl; i++) repNurl(set[nr].nurl[i]);
      for (i=0; i<set[nr].anz_nurs; i++) repNurs(set[nr].nurs[i]);
      for (i=0; i<set[nr].anz_nurs; i++) untrimNurs(set[nr].nurs[i]);
      for (i=0; i<set[nr].anz_s; i++) repSurf(set[nr].surf[i],1);
    }
    /* reposition the internal drawing nodes */
    if(anz->e>0)
    {
      adjustDrawNodes(0);
      //makeSurfaces();
      updateDispLists();
    }
    i=getSetNr(specialset->bsur);
    if(i>-1) if(set[i].anz_s)
      printf("WARNING: %d surfaces could not be filled, see set:%s\n", set[i].anz_s,specialset->bsur);
  }
  else if (compareStrings(type, "RNAM")>0)
  {
    sscanf(string, "%*s %s%s", name, xbuf );
    setNr=getSetNr(xbuf);
    if (setNr>-1)
    {
      printf ("ERROR: set:%s exist already\n", xbuf);
      goto checkForError;;
    }

    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf ("ERROR: set:%s does not exist\n", name);
    }
    else
    {
      printf(" rename %s", set[setNr].name );
      rnam(setNr, xbuf ); 
      printf(" to %s \n ", set[setNr].name ); 
    }
  }
  else if (compareStrings(type, "ROT")>0)
  {
    pre_rot( &string[na+1] );
    // redraw re-sets an eventual sequence
    redraw();
  }
  else if ( compareStrings(type, "SAVE")>0 )
  {
    strcpy(setname,datin);
    for (i=strlen(datin); i>=0; i--)
    {
      if(datin[i]=='.')break;
    }
    setname[i]='\0';
#ifdef WIN32
    sprintf (buffer, "move /y \"%s\" \"%s.fbb\"", datin, setname );
#else
    sprintf (buffer, "mv %s %s.fbb", datin, setname );
#endif
    if(compare(&datin[i+1],"fbd",3)==3) system (buffer);
    pre_write(" all fbd ");
    if( compareStrings( setname, "all")<1 )
    {
#ifdef WIN32
      sprintf (buffer, "move /y all.fbd \"%s.fbd\"", setname );
#else
      sprintf (buffer, "mv all.fbd %s.fbd", setname );
#endif
      system (buffer);
    }     
  }
  else if ( compareStrings(type, "SCAL")>0 )
  {
    entity[0]=0; x=0.;
    length=sscanf(&string[na+1],"%s %lf", entity, &x);
    /* vector length */
    if(entity[0]=='v')
    {
      if(x!=0.)  v_scale*=x;
      else v_scale=1.;
      updateDispLists();
    }
    else if(entity[0]=='d') 
    {
      if(x!=0.) anim_faktor*=x;
      else  anim_faktor=1.;
      if(addDispFlag==1)
      {
         addDispToCoordinates(node);
         addDispToCoordinates(node);
      }
      redraw();
    }
    else if(entity[0]=='s') 
    {
      scale->smin*=x; scale->smax*=x;
      redraw();
    }
    else printf("type:%s not known, factor:%lf\n", entity, x);
  }
  else if (compareStrings(type, "SEND")>0)
  {
    pre_write( &string[na+1] );
  }
  else if (compareStrings(type, "SEQA")>0)
  {
    /* if the name is in use and no valid data are spec. then the seq will be overwritten */
    /* if the name is in use and valid data are spec. then the command is ignored */

    /* for the moment the commands AFTE and BEFO are wrong interpreted */

    /* determine the commands */
    na= sscanf( string, "%*s %s%s%s%s ", name, dat[0],dat[1],dat[2]);
    /* search for valid commands */
    if((dat[0][0]=='A')||(dat[0][0]=='a'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 4) goto checkForError;
      strcpy(comm,"AFTE"); 
      if (addFlag) { for(j=strlen(dat[1]); j>=0; j--) dat[1][j+1]=dat[1][j]; dat[1][0]='%'; }
      operateAlias( dat[1], "p" );
      strcpy(posn,dat[1]); 
      strcpy(entity,dat[2]);
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s");
    }
    else if((dat[0][0]=='B')||(dat[0][0]=='b'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 4) goto checkForError;
      strcpy(comm,"BEFO"); 
      if (addFlag) { for(j=strlen(dat[1]); j>=0; j--) dat[1][j+1]=dat[1][j]; dat[1][0]='%'; }
      operateAlias( dat[1], "p" );
      strcpy(posn,dat[1]); 
      strcpy(entity,dat[2]);
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }
    else if ((dat[0][0]=='E')||(dat[0][0]=='e'))
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='%'; }
      operateAlias( name, "se" );
      if( na < 3) goto checkForError;
      strcpy(comm,"END"); 
      strcpy(entity,dat[1]); 
      strcpy(format
      , "%*s%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }
    else
    {
      if (addFlag) { for(j=strlen(name); j>=0; j--) name[j+1]=name[j]; name[0]='!'; }
      operateAlias( name, "se" );
      if( na < 3) goto checkForError;
      strcpy(comm,"NEW"); 
      strcpy(entity,dat[0]); 
      strcpy(format
      , "%*s%*s%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
    }

    j=anz_seq=0;
  newline:;
    na =   sscanf( string,format
              , dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9]
              , dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);
    anz_seq+=na;
    if( (seq = (char **)realloc((char **)seq, (anz_seq)*sizeof(char *))) == NULL )
    { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
    for (i=anz_seq-na; i<anz_seq; i++)
    {
      if( (seq[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
      { printf(" ERROR: malloc failure in interpreter()\n\n"); return(-1); }
    }

    /* look if the command-line continues */
    for (i=0; i<na; i++)
    {
      /* printf("dat:%d |%s|\n",i, dat[i]); */
      if(dat[i][0]=='=')
      {
        length = frecord( handle1, string);
        strcpy(format
        , "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"); 
        goto newline;
      }
      if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
      operateAlias( dat[i], "p" );
      strcpy(seq[j], dat[i]);
      j++;
    }

    if((entity[0]=='N')||entity[0]=='n') type[0]='n';
    else if((entity[0]=='P')||entity[0]=='p') type[0]='p';
    else 
    {
      errMsg(" ERROR in SEQA: entity not known\n" );
      goto checkForError;
    }
    /* mark the set as a sequence-set */
    type[1]='s';

    if(comm[0]=='A')
    {
      /* the set exists */
      setNr=getSetNr( name );
      if (setNr<0)
      {
        printf (" ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }

      if(type[0]=='p') ibuf=set[setNr].anz_p+anz_seq;
      if(type[0]=='n') ibuf=set[setNr].anz_n+anz_seq;
      if((buf=(char **)malloc(ibuf*sizeof(char *)))==NULL)
      { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      for (i=0; i<ibuf; i++)
      {
        if( (buf[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
        { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      }

      j=0;
      if( type[0]=='p')
      {
       for (i=0; i<set[setNr].anz_p; i++)
       {
        strcpy( buf[j], point[set[setNr].pnt[i]].name); j++;
        if( compareStrings( point[set[setNr].pnt[i]].name, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
       }
       set[setNr].anz_p = 0;
      }     
      if( type[0]=='n')
      {
       for (i=0; i<set[setNr].anz_n; i++)
       {
        sprintf( buf[j],"%d", set[setNr].node[i]);
        if( compareStrings( buf[j], posn)>0)
        {
          for (k=0; k<anz_seq; k++) { j++; strcpy( buf[j], seq[k]); }
        }
        j++;
       }
       set[setNr].anz_n = 0;
      }     
      for (i=0; i<j; i++)
      {
        pre_seta( name, type, buf[i]);
      }

      for (i=0; i<ibuf; i++) free(buf[i]); free(buf);
    }
    else if(comm[0]=='B')
    {
      /* the set exists */
      setNr=getSetNr( name );
      if (setNr<0)
      {
        printf (" ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }

      if(type[0]=='p') ibuf=set[setNr].anz_p+anz_seq;
      if(type[0]=='n') ibuf=set[setNr].anz_n+anz_seq;
      if((buf=(char **)malloc(ibuf*sizeof(char *)))==NULL)
      { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      for (i=0; i<ibuf; i++)
      {
        if( (buf[i]= (char *)malloc( MAX_LINE_LENGTH*sizeof(char))) == NULL )
        { printf(" ERROR: realloc failure in interpreter()\n\n"); return(-1); }
      }

      j=0;
      if( type[0]=='p')
      {
       for (i=0; i<set[setNr].anz_p; i++)
       {
        if( compareStrings( point[set[setNr].pnt[i]].name, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
        strcpy( buf[j], point[set[setNr].pnt[i]].name); j++;
       }
       set[setNr].anz_p = 0;
      }     
      if( type[0]=='n')
      {
       for (i=0; i<set[setNr].anz_n; i++)
       {
        sprintf( xbuf,"%d", set[setNr].node[i]);
        if( compareStrings( xbuf, posn)>0)
        {
          for (k=0; k<anz_seq; k++) { strcpy( buf[j], seq[k]); j++; }
        }
        sprintf( buf[j],"%d", set[setNr].node[i]); j++;
       }
       set[setNr].anz_n = 0;
      }     
      for (i=0; i<j; i++)
      {
        pre_seta( name, type, buf[i]);
      }

      for (i=0; i<ibuf; i++) free(buf[i]); free(buf);
    }
    else if(comm[0]=='E')
    {
      for (i=0; i<anz_seq; i++)
      {
        pre_seta( name, type, seq[i]);
      }
    }
    else if(comm[0]=='N')
    {
      delSet(name);
      for (i=0; i<anz_seq; i++)
      {
        pre_seta( name, type, seq[i]);
      }
    }

    for (i=0; i<anz_seq; i++) free(seq[i]);
  }
  else if (compareStrings(type, "SEQL")>0)
  {
    k=0;
    na= sscanf( string, "%*s %s %d", name, &k);
    setNr=getSetNr(name);
    if (setNr<0)
    {
      printf (" set:%s does not exist\n", name);
      goto checkForError;
    }
    if(k<1)
    {
      printf (" the nr of spline-points must be greater than 0\n");
      goto checkForError;
    }
    seto(set[setNr].name);
    for(i=0; i<set[setNr].anz_l; i++) convertLine( set[setNr].line[i],k);
    setc(set[setNr].name);
  }
  else if (compareStrings(type, "SETA")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    /* create sets for disjunct meshes */ 
    if((strlen(name)==1)&&(name[0]=='!'))
    {
      if(type[0]=='e') separateMeshes(dat[0]);
      else if(type[0]=='l') separateLines(dat[0]);
      else separateMeshes(type);
      goto checkForError;
    }

    /* check if the setname matches a seqential-set (sq). if yes do not add entities */
    l=getSetNr(name);
    if (l>-1)
    {
      if(set[l].type==1)
      {
        printf("ERROR: setname:%s in use by a sequence. Set can not be created.\n", name);
        goto checkForError;
      }
    }

    /* Single letters (except 'se' 'sh' 'ld') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='v')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='d'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      /* look if a sequence is defined */
      if((dat[1][0]=='-')&&(dat[1][1]=='\0'))
      {
        if((type[0]!='n')&&(type[0]!='e')) 
        {
          printf("ERROR: sequence only possible for nodes and elements. Set %s can not be created.\n", name);
          goto checkForError;
        }
        for(i=atoi(dat[0]); i<=atoi(dat[2]); i+=atoi(dat[3]) )
        {
          sprintf(buffer,"%d",i);
          pre_seta( name, type, buffer);
        }
      }
      else
      {
        for (i=0; i<na-2; i++)
        {
          if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
          operateAlias( dat[i], type );

          /* try to add entity of type to the set, if it fails the entity might be a set itself */
          if( pre_seta( name, type, dat[i]) == -1)
	  {
            /* look if a set of this name exists */
            nr=getSetNr(dat[i]);
            if(nr<0)
            {
              /* set was not found. check if wildcards (*) were used */
              length= strsplt( dat[i], '*', &substring);
              if ((length>0)&&(strstr(dat[i], "*") !=NULL))
              {
                for(setNr=0; setNr<anz->sets; setNr++) if(set[setNr].name!=(char *)NULL)
                {
                  foundSubString=0;
                  for(l=0; l<length; l++)
                  {
                    if(strstr(set[setNr].name, substring[l]) !=NULL)
                    {
                      foundSubString++;
                      /* check if the first or the last char is no '*' then the substring[] must be at start or end */
          	      if(l==0) { if(dat[i][0]!='*')  { if(dat[i][0]!=set[setNr].name[0])  foundSubString--; }  }
               	      if(l==length-1) { if(dat[i][strlen(dat[i])-1]!='*') { if(dat[i][strlen(dat[i])-1]!=set[setNr].name[strlen(set[setNr].name)-1])  foundSubString--; } }
            	    }
            	  }
                  if(foundSubString==length)
                  {
                    nr=setNr;
	            printf(" set:%s matches %s\n", set[nr].name, dat[i]);
                    if( pre_seta( name, type, set[nr].name ) == -1)
	            {
                        k=pre_seta( name, "i", 0);
                        if ( type[0] == 'n' )        for(j=0; j<set[nr].anz_n; j++) seta(k,type,set[nr].node[j]);
                        else if ( type[0] == 'e' )   for(j=0; j<set[nr].anz_e; j++) seta(k,type,set[nr].elem[j]);
                        else if ( type[0] == 'f' )   for(j=0; j<set[nr].anz_f; j++) seta(k,type,set[nr].face[j]);
                        else if ( type[0] == 'v' )   for(j=0; j<set[nr].anz_v; j++) seta(k,type,set[nr].valu[j]);
                        else if ( type[0] == 'p' )   for(j=0; j<set[nr].anz_p; j++) seta(k,type,set[nr].pnt[j]);
                        else if ( type[0] == 'l' )
			{
                          if((type[1]=='d')&&(type[2]!='\0'))
			  {
                            for(j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].div>=atoi(&type[2])) seta(k,type,set[nr].line[j]);
			  }
			  else if(type[1]!='\0') { for(j=0; j<set[nr].anz_l; j++) seta(k,type,set[nr].line[j]); }
			}
                        else if ( type[0] == 'c' )   for(j=0; j<set[nr].anz_c; j++) seta(k,type,set[nr].lcmb[j]);
                        else if (( type[0] == 's' )&&(type[1]=='\0'))   for(j=0; j<set[nr].anz_s; j++) seta(k,type,set[nr].surf[j]);
                        else if (( type[0] == 's' )&&(type[1]=='h'))   for(j=0; j<set[nr].anz_sh; j++) seta(k,type,set[nr].shp[j]);
                        else if ( type[0] == 'b' )   for(j=0; j<set[nr].anz_b; j++) seta(k,type,set[nr].body[j]);
                        else if ( type[0] == 'L' )   for(j=0; j<set[nr].anz_nurl; j++) seta(k,type,set[nr].nurl[j]);
                        else if ( type[0] == 'S' )   for(j=0; j<set[nr].anz_nurs; j++) seta(k,type,set[nr].nurs[j]);
                    }
                  }
                }
              }
              /* free substring */
              for(j=0; j<length; j++) free(substring[j]);
              free(substring);
            }
            else
            {
		//printf(" entity:%s of type:%c does not exist add contents of set:%d\n", dat[i], type[0], nr);
              k=pre_seta( name, "i", 0);
	      if ( type[0] == 'n' )        for(j=0; j<set[nr].anz_n; j++) seta(k,type,set[nr].node[j]);
	      else if ( type[0] == 'e' )   for(j=0; j<set[nr].anz_e; j++) seta(k,type,set[nr].elem[j]);
	      else if ( type[0] == 'f' )   for(j=0; j<set[nr].anz_f; j++) seta(k,type,set[nr].face[j]);
 	      else if ( type[0] == 'v' )   for(j=0; j<set[nr].anz_v; j++) seta(k,type,set[nr].valu[j]);
 	      else if ( type[0] == 'p' )   for(j=0; j<set[nr].anz_p; j++) seta(k,type,set[nr].pnt[j]);
              else if ( type[0] == 'l' )
	      {
                if((type[1]=='d')&&(type[2]!='\0'))
	        {
                  for(j=0; j<set[nr].anz_l; j++) if(line[set[nr].line[j]].div>=atoi(&type[2])) seta(k,type,set[nr].line[j]);
	        }
	        else if(type[1]=='\0') { for(j=0; j<set[nr].anz_l; j++) seta(k,type,set[nr].line[j]); }
		else printf(" ERROR: Parameter:%s unknown\n",type);
	      }
 	      else if ( type[0] == 'c' )   for(j=0; j<set[nr].anz_c; j++) seta(k,type,set[nr].lcmb[j]);
 	      else if (( type[0] == 's' )&&(type[1]=='\0'))   for(j=0; j<set[nr].anz_s; j++) seta(k,type,set[nr].surf[j]);
 	      else if (( type[0] == 's' )&&(type[1]=='h'))   for(j=0; j<set[nr].anz_sh; j++) seta(k,type,set[nr].shp[j]);
 	      else if ( type[0] == 'b' )   for(j=0; j<set[nr].anz_b; j++) seta(k,type,set[nr].body[j]);
 	      else if ( type[0] == 'L' )   for(j=0; j<set[nr].anz_nurl; j++) seta(k,type,set[nr].nurl[j]);
 	      else if ( type[0] == 'S' )   for(j=0; j<set[nr].anz_nurs; j++) seta(k,type,set[nr].nurs[j]);
	    }
          }
        }
      }

      if((type[0]=='s')&&(type[1]=='e'))
      {
        completeSet( name, "do");
      }
    }
    else
    {
      /* items of unknown type, determine each type */
      if (addFlag) { for(l=strlen(type); l>=0; l--) type[l+1]=type[l]; type[0]='%'; }
      ibuf=0;
      operateAlias( type, "v" ); if (getValuNr(type) >-1)  {  pre_seta( name, "v", type); goto next_seta;  } 
      operateAlias( type, "b" ); if (getBodyNr(type) >-1)  {  pre_seta( name, "b", type); goto next_seta;  } 
      operateAlias( type, "s" ); if (getSurfNr(type) >-1)  {  pre_seta( name, "s", type); goto  next_seta;  } 
      operateAlias( type, "c" ); if (getLcmbNr(type) >-1)  {  pre_seta( name, "c", type); goto  next_seta;  } 
      operateAlias( type, "l" ); if (getLineNr(type) >-1)  {  pre_seta( name, "l", type); goto  next_seta;  } 
      operateAlias( type, "p" ); if (getPntNr(type) >-1)   {  pre_seta( name, "p", type); goto  next_seta;  } 
      operateAlias( type, "se" ); if (getSetNr(type) >-1)   {  pre_seta( name, "se", type); ibuf=1; goto  next_seta;  } 
      operateAlias( type, "sh" ); if (getShapeNr(type) >-1) {  pre_seta( name, "sh", type); ibuf=1; goto  next_seta;  } 
      next_seta:;

      for (i=0; i<na-2; i++)
      {
        if (addFlag) { for(l=strlen(dat[i]); l>=0; l--) dat[i][l+1]=dat[i][l]; dat[i][0]='%'; }
        operateAlias( dat[i], "v" ); if (getValuNr(dat[i]) >-1)  { pre_seta( name, "v",dat[i] ); goto next_dat;  } 
        operateAlias( dat[i], "b" ); if (getBodyNr(dat[i]) >-1)  { pre_seta( name, "b",dat[i] ); goto next_dat;  } 
        operateAlias( dat[i], "s" ); if (getSurfNr(dat[i]) >-1)  { pre_seta( name, "s",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "c" ); if (getLcmbNr(dat[i]) >-1)  { pre_seta( name, "c",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "l" ); if (getLineNr(dat[i]) >-1)  { pre_seta( name, "l",dat[i] ); goto  next_dat;  }
        operateAlias( dat[i], "p" ); if (getPntNr(dat[i]) >-1)   { pre_seta( name, "p",dat[i] ); goto  next_dat;  } 
        operateAlias( dat[i], "se" ); if (getSetNr(dat[i]) >-1)   {  pre_seta( name, "se",dat[i]); ibuf=1; goto  next_dat;  } 
        operateAlias( dat[i], "sh" ); if (getShapeNr(dat[i]) >-1) {  pre_seta( name, "sh",dat[i]); ibuf=1; goto  next_dat;  } 
      next_dat:;
      }
      if(ibuf)
      {
        completeSet( name, "do");
      }
    }
  }
  else if (compareStrings(type, "SETC")>0)
  {
    for(i=0; i<MAX_LINE_LENGTH; i++) name[i]=0;
    length=sscanf( string, "%*s%s", name);
    setc(name);
  }
  else if (compareStrings(type, "SETE")>0)
  {
    for(i=0; i<MAX_LINE_LENGTH; i++) name[i]=0;
    strcpy(comm,"strict");
    length=sscanf( string, "%*s%s%s%s", name, type, comm);

    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      setNr=getSetNr(name);
      if (setNr==-1)
      {
        printf("ERROR: set:%s does not exist\n", name);
        goto checkForError;
      }
      sete(setNr,type,comm);
    }
    else
    {
      printf(" ERROR: Type of data to be enquired has to be stated\n");
      goto checkForError;
    }
  }
  else if (compareStrings(type, "SETI")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    /* check if the setname matches a seqential-set (sq). if yes do not add entities */
    l=getSetNr(name);
    if (l>-1)
    {
      if(set[l].type==1)
      {
        printf("ERROR: setname:%s in use by a sequence. Set can not be created.\n", name);
        goto checkForError;
      }
    }
    for (i=0; i<na-2; i++)
    {
      operateAlias( dat[i], type );
      l=getSetNr(dat[i]);
      if (l==-1)
      {
        printf("ERROR: set:%s does not exist\n", dat[i]);
        goto checkForError;
      }
    }

    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='r')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      /* add the spec type to set 'name' if its included in all spec. sets (dat) */
      setNr=pre_seta( name, "i", 0);
      seti(setNr,type,na-2, dat);
    }
    else
    {
      printf(" ERROR: Type of data to be intersected has to be stated\n");
      goto checkForError;
    }
  }
  else if (compareStrings(type, "SETO")>0)
  {
    name[0]='\0';
    length=sscanf( string, "%*s%s", name);
    seto(name);
  }
  else if (compareStrings(type, "SETR")>0)
  {
    na =   sscanf( string, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
               name, type, dat[0],dat[1],dat[2],dat[3],dat[4],dat[5], dat[6],dat[7],dat[8],dat[9],
               dat[10],dat[11],dat[12],dat[13],dat[14],dat[15], dat[16],dat[17],dat[18],dat[19]);

    operateAlias( name, "se" );
    setNr=getSetNr( name );
    if (setNr<0)
    {
      errMsg(" ERROR: Set (%s) is undefined\n", name );
      goto checkForError;
    }
    /* Single letters (except 'se' 'sh') determine the type of the following entities in cgx. */
    /* In FAM no indication of the type is given. So if the name of the entity */
    /* consists only of a single letter (or 'se'), it will be ignored. */ 
    if (((type[0]=='n')&&(type[1]=='\0'))||
        ((type[0]=='e')&&(type[1]=='\0'))||
        ((type[0]=='f')&&(type[1]=='\0'))||
        ((type[0]=='v')&&(type[1]=='\0'))||
        ((type[0]=='p')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='\0'))||
        ((type[0]=='l')&&(type[1]=='s'))||
        ((type[0]=='c')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='\0'))||
        ((type[0]=='s')&&(type[1]=='e'))||
        ((type[0]=='s')&&(type[1]=='h'))||
        ((type[0]=='L')&&(type[1]=='\0'))||
        ((type[0]=='S')&&(type[1]=='\0'))||
        ((type[0]=='b')&&(type[1]=='\0')) )
    {
      for (i=0; i<na-2; i++)
      {
        operateAlias( dat[i], type );
        nr=getSetNr(dat[i]);
        if(nr>-1)
        {
          if ((type[0]=='s')&&(type[1]=='e'))
          {
            /* remove the contents of nr from setNr */
            for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
            for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
            for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
            for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
            for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );
            for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
            for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
            for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] );
            for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );
            for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
            for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
            for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
            for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
          }
          else if (type[0]=='v') for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
          else if (type[0]=='n') for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
          else if (type[0]=='e') for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
          else if (type[0]=='f') for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
          else if (type[0]=='p') for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );	
          else if (type[0]=='l')
	  {
            for (j=0; j<set[nr].anz_l; j++)
	    {
              if(type[1]=='s') if(line[set[nr].line[j]].typ!='s') continue;
              setr( setNr, "l", set[nr].line[j] );
	    }
	  }
          else if (type[0]=='c') for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
          else if ((type[0]=='s')&&(type[1]=='\0')) for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] ); 
          else if (type[0]=='b') for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );   
          else if (type[0]=='L') for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
          else if (type[0]=='S') for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
          else if ((type[0]=='s')&&(type[1]=='h')) for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
          else if (type[0]=='r') for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
        }
        else
	{
          if(strstr(dat[i], "*") !=NULL)
          {
            /* set was not found. check if wildcards (*) were used */
            length= strsplt( dat[i], '*', &substring);
            if (length>0)
            {
              for(nr=0; nr<anz->sets; nr++) if(set[nr].name!=(char *)NULL)
              {
                foundSubString=0;
                for(l=0; l<length; l++)
                {
                  if(strstr(set[nr].name, substring[l]) !=NULL)
                  {
                    foundSubString++;
                    /* check if the first or the last char is no '*' then the substring[] must be at start or end */
        	      if(l==0) { if(dat[i][0]!='*')  { if(dat[i][0]!=set[nr].name[0])  foundSubString--; }  }
             	      if(l==length-1) { if(dat[i][strlen(dat[i])-1]!='*') { if(dat[i][strlen(dat[i])-1]!=set[nr].name[strlen(set[nr].name)-1])  foundSubString--; } }
                  }
          	}
                if(foundSubString==length)
                {
	          printf(" set:%s matches %s\n", set[nr].name, dat[i]);

                  operateAlias( set[nr].name, type );
                  nr=getSetNr(set[nr].name);
                  if(nr>-1)
                  {
                    if ((type[0]=='s')&&(type[1]=='e'))
                    {
                      /* remove the contents of nr from setNr */
                      for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
                      for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
                      for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
                      for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
                      for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );
                      for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
                      for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
                      for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] );
                      for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );
                      for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
                      for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
                      for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
                      for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
                    }
                    else if (type[0]=='v') for (j=0; j<set[nr].anz_v; j++) setr( setNr, "v", set[nr].valu[j] );
                    else if (type[0]=='n') for (j=0; j<set[nr].anz_n; j++) setr( setNr, "n", set[nr].node[j] );
                    else if (type[0]=='e') for (j=0; j<set[nr].anz_e; j++) setr( setNr, "e", set[nr].elem[j] );
                    else if (type[0]=='f') for (j=0; j<set[nr].anz_f; j++) setr( setNr, "f", set[nr].face[j] );
                    else if (type[0]=='p') for (j=0; j<set[nr].anz_p; j++) setr( setNr, "p", set[nr].pnt[j] );	
                    else if (type[0]=='l') for (j=0; j<set[nr].anz_l; j++) setr( setNr, "l", set[nr].line[j] );
                    else if (type[0]=='c') for (j=0; j<set[nr].anz_c; j++) setr( setNr, "c", set[nr].lcmb[j] );
                    else if ((type[0]=='s')&&(type[1]=='\0')) for (j=0; j<set[nr].anz_s; j++) setr( setNr, "s", set[nr].surf[j] ); 
                    else if (type[0]=='b') for (j=0; j<set[nr].anz_b; j++) setr( setNr, "b", set[nr].body[j] );   
                    else if (type[0]=='L') for (j=0; j<set[nr].anz_nurl; j++) setr( setNr, "L", set[nr].nurl[j] );
                    else if (type[0]=='S') for (j=0; j<set[nr].anz_nurs; j++) setr( setNr, "S", set[nr].nurs[j] );
                    else if ((type[0]=='s')&&(type[1]=='h')) for (j=0; j<set[nr].anz_sh; j++) setr( setNr, "sh", set[nr].shp[j] );
                    else if (type[0]=='r') for (j=0; j<set[nr].anz_se; j++) setr( setNr, "r", set[nr].set[j] );
                  }
                }
              }
              /* free substring */
              for(i=0; i<length; i++) free(substring[i]);
              free(substring);
            }
          }
          else
	  {
            if (type[0]=='n') nr=atoi(dat[i]);
            else if (type[0]=='e') nr=atoi(dat[i]);
            else if (type[0]=='f') nr=atoi(dat[i]);
            else if (type[0]=='v') nr=getValuNr(dat[i]);
            else if (type[0]=='p') nr=getPntNr(dat[i]);
            else if (type[0]=='l') nr=getLineNr(dat[i]);
            else if (type[0]=='c') nr=getLcmbNr(dat[i]);
            else if ((type[0]=='s')&&(type[1]=='\0')) nr=getSurfNr(dat[i]);
            else if (type[0]=='b') nr=getBodyNr(dat[i]);
            else if (type[0]=='L') nr=getNurlNr(dat[i]);
            else if (type[0]=='S') nr=getNursNr(dat[i]);
            else if ((type[0]=='s')&&(type[1]=='h')) nr=getShapeNr(dat[i]);
            else if (type[0]=='r') nr=getSetNr(dat[i]);
            setr( setNr, type, nr);
	  }
        }
      }
    }
    else
    {
      /* items of unknown type, determine each type */
      if (getValuNr(type) >-1)
      { setr( setNr, "v", getValuNr(type)); } 
      if (getBodyNr(type) >-1)
      { operateAlias( type, "b" ); setr( setNr, "b", getBodyNr(type)); } 
      if (getSurfNr(type) >-1)
      { operateAlias( type, "s" ); setr( setNr, "s", getSurfNr(type)); } 
      if (getLcmbNr(type) >-1)
      { operateAlias( type, "c" ); setr( setNr, "c", getLcmbNr(type)); } 
      if (getLineNr(type) >-1)
      { operateAlias( type, "l" ); setr( setNr, "l", getLineNr(type)); } 
      if (getPntNr(type) >-1)
      { operateAlias( type, "p" ); setr( setNr, "p", getPntNr(type)); } 
      if (getShapeNr(type) >-1)
      { operateAlias( type, "sh" ); setr( setNr, "sh", getShapeNr(type)); } 
      for (i=0; i<na-2; i++)
      {
        if (getValuNr(dat[i]) >-1)
        { setr( setNr, "v", getValuNr(dat[i]) ); } 
        if (getBodyNr(dat[i]) >-1)
        { operateAlias( dat[i], "b" ); setr( setNr, "b", getBodyNr(dat[i]) ); } 
        if (getSurfNr(dat[i]) >-1)
        { operateAlias( dat[i], "s" ); setr( setNr, "s", getSurfNr(dat[i]) ); } 
        if (getLcmbNr(dat[i]) >-1)
        { operateAlias( dat[i], "c" ); setr( setNr, "c", getLcmbNr(dat[i]) ); } 
        if (getLineNr(dat[i]) >-1)
        { operateAlias( dat[i], "l" ); setr( setNr, "l", getLineNr(dat[i]) ); }
        if (getPntNr(dat[i]) >-1)
        { operateAlias( dat[i], "p" ); setr( setNr, "p", getPntNr(dat[i]) ); } 
        if (getShapeNr(dat[i]) >-1)
        { operateAlias( dat[i], "sh" ); setr( setNr, "sh", getShapeNr(dat[i]) ); } 
      }
    }
  }
  else if (compareStrings(type, "SHPE")>0)
  {
    pre_shape( &string[na+1], addFlag);
  }
  else if (compareStrings(type, "SPLIT")>0)
  {
    pre_split( &string[na+1] );
    redraw();
  }
  else if (compareStrings(type, "STACK")>0)
  {
    if(compare( &string[na+1],"on",2)==2 ) valuestackFlag=1;
    if(compare( &string[na+1],"off",2)==2 ) valuestackFlag=0;
    if(compare( &string[na+1],"free",2)==2 )
    {
      for(i=0; i<valuestack_ptr; i++)  free(valuestack[i]);
      valuestack_ptr=0;
      free(valuestack);
      valuestack=NULL;
    }
  }
  else if (compareStrings(type, "STEPS")>0)
  {
    steps= atoi( &string[na+1]  );
#if INX_MODE
    calcOffset();
    maxIndex= offset+steps-1;
#endif
    redraw();
  }
  else if (compareStrings(type, "SURF")>0)
  {
    pre_surf(&string[na+1]);
  }
  else if (compareStrings(type, "SWEP")>0)
  {
    pre_swep(&string[na+1]);
    redraw();
  }
  else if (compareStrings(type, "SYS")>0)
  {
    system(&string[na+1]);
  }
  else if (compareStrings(type, "TEST")>0)
  {
    length=sscanf( string, "%*s %s %s", type, name);
    operateAlias( name, type );
    strcpy(parameter[0],"FALSE");
    if ((type[0]=='n')&&(type[1]=='\0')) { ibuf=atoi(name);
      if ((ibuf>=anz->nmin)&&(ibuf<=anz->nmax)&&(node[ibuf].indx==0)&&(ibuf==node[node[ibuf].indx].nr)&&(node[ibuf].pflag==0)) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='e')&&(type[1]=='\0')) { ibuf=atoi(name);
      if ((ibuf>=anz->emin)&&(ibuf<=anz->emax)&&(e_enqire[ibuf].type!=0)) strcpy(parameter[0],"TRUE"); }
    //if ((type[0]=='f')&&(type[1]=='\0')) { if( get??Nr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='v')&&(type[1]=='\0')) { if( getValuNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='p')&&(type[1]=='\0')) { if( getPntNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='l')&&(type[1]=='\0')) { if( getLineNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='c')&&(type[1]=='\0')) { if( getLcmbNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    //if ((type[0]=='r')&&(type[1]=='\0')) { if( get??Nr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='\0')) { if( getSurfNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='e'))  { if( getSetNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='s')&&(type[1]=='h'))  { if( getShapeNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='L')&&(type[1]=='\0')) { if( getNurlNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='S')&&(type[1]=='\0')) { if( getNursNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    if ((type[0]=='b')&&(type[1]=='\0')) { if( getBodyNr(name) >-1) strcpy(parameter[0],"TRUE"); }
    printf(" %s\n",parameter[0]);
    write2stack(1, parameter);
  }
  else if (compareStrings(type, "TEXT")>0)
  {
    stos( &string[na], 1, strlen(&string[na]), picture_text);
  }
  else if (compareStrings(type, "TRA")>0)
  {
    length=sscanf( string, "%*s %s %lf", posn, &dx);
    if(posn[0]=='u') dty+= dx/scale->w/ds;
    if(posn[0]=='d') dty-= dx/scale->w/ds;
    if(posn[0]=='l') dtx-= dx/scale->w/ds;
    if(posn[0]=='r') dtx+= dx/scale->w/ds;
    if(posn[0]=='f') dtz+= dx/scale->w;
    //printf("dtx:%f dty:%f dtz:%f dx:%f ds:%f\n", dtx, dty, dtz, (dx*2)/scale->w, ds);
    glutPostRedisplay();
    updateDispLists();
  }
  else if (compareStrings(type, "TRFM")>0)
  {
    transformResults(&string[na+1]);
  }
  else if (compareStrings(type, "UCUT")>0)
  {
    uncut(1);
  }
  else if (compareStrings(type, "VALU")>0)
  {
    pre_value(&string[na+1]);
  }
  else if (compareStrings(type, "VIEW")>0)
  {
    pre_view(&string[na+1]);
  }
  else if (compareStrings(type, "VOLU")>0)
  {
    pre_volu(&string[na+1]);
  }
  else if (compareStrings(type, "ZAP")>0)
  {
    if( zap(&string[na+1])==-1)
      printf (" ERROR in ZAP: set:%s does not exist\n",&string[na+1] );
  }
  else if (compareStrings(type, "ZOOM")>0)
  {
    length=sscanf( string, "%*s %lf %lf %lf %lf", &wx[0], &wy[0], &wx[1], &wy[1]);
    if (length==4)
    {
      wx[0]*=2.;
      wx[1]*=2.;
      wy[0]*=2.;
      wy[1]*=2.;

      wx[0]+=-1.;
      wx[1]+=-1.;
      wy[0]+=-1.;
      wy[1]+=-1.;
    }
    else
    {
      dx=1./wx[0];
      wx[0]=wy[0]=-dx;
      wx[1]=wy[1]=dx;
    }
    zoom(wx[0], wy[0], wx[1], wy[1]);
  }
  else if ((type[0]== '!')||(type[0]== '#')||(type[0]== '$'))
  {
    printf(" %s\n", string );
  }
  else
  {
    printf(" key:%s from string %s not known\n", type, string);
  }

 checkForError:;
  if(returnFlag<0) return(returnFlag);
  if(whileFlag==3) goto whileLoop;
  return(returnFlag);
}



void readfbd( char *datin, int addFlag )
{
  FILE      *handle1;
  int       result=0, length, i, na, gtolFlag=0;
  char type[MAX_LINE_LENGTH];
  static char *string;
  static char buffer[MAX_LINE_LENGTH];
  int valuestackFlagbuffer=0;

  handle1 = fopen (datin, "r");
  if (handle1==NULL)
  {
    printf (" ERROR: The input file \"%s\" could not be opened.\n\n", datin);
    return;
  }
  else  printf ("\n%s opened",datin);

  printf ("\n reading file\n");
  while (result > -1)
  {
    /* for long commands (>MAX_LINE_LENGTH-1) combine several calls to frecord to build the command which will be stored in 'string' */
    i=0;
  addLine:;
    do
    {
      if( (string=realloc(string, MAX_LINE_LENGTH*++i*sizeof(char)))== NULL ) { printf(" ERROR: realloc failed in readfbd()\n"); return; }
      length = frecord( handle1, buffer);
      //strcpy(&string[(MAX_LINE_LENGTH-1)*(i-1)], buffer);
      if(i==1) strcpy(string, buffer);
      else strcpy(&string[strlen(string)], buffer);
    }while(length==(MAX_LINE_LENGTH-1));
    if((length>0)&&(string[length-1]=='='))
    {
      string[length-1]=0;
      goto addLine;
    }
    if (length > 0)
    {
      type[0]=0;
      na=0; while(string[na]==' ') na++;
      na+= sword( string, type);
      /* printf ("\n record:%s", &string[0]); */
      result=commandoInterpreter( type, string, na, length, handle1, addFlag, &gtolFlag );
    }
    if( string[length]== (char)EOF)  break;
  }
  fclose( handle1);

  if(!gtolFlag) { valuestackFlagbuffer=valuestackFlag; valuestackFlag=0; gtol=calcGTOL(setall); valuestackFlag=valuestackFlagbuffer; }

  /* delete the entities in specialSet->zap */
  zap(specialset->zap);

  /* fit everything in the drawing cube and repaint the entities */
  /* (should be done in the trigger functions like move() */
  if((!animFlag)&&(!sequenceFlag))
  {
  descalAll();
  getScaleValues( setall, set, point, node, scale);
  scalNodes ( anz->n, node, scale );
  scalPoints ( anzGeo->p, point, scale );
  scalSurfs( anzGeo->s, surf, scale);
  // recalculate the line-shapes
  for (i=0; i<anzGeo->l; i++) repLine(i);
  // recalculate the nurbl-controll-points
  for (i=0; i<anzGeo->nurl; i++) repNurl(i);
  // recalculate the nurbs-controll-points
  for (i=0; i<anzGeo->nurs; i++) repNurs(i);
  // correct the orientation of all entities
  orientSet( "all" );
  }

  printf (" done \n\n");
}


/*------------------------------------------------------------------*/
/* Daten im frd-format einlesen                                     */
/*------------------------------------------------------------------*/
void readfrdfile( char *frdfile, char *setname )
{
  int i, setNr=-1;
  char  datum[MAX_LINE_LENGTH];
  Summen    anzr[1];
  Nodes     *noder=NULL;
  Elements  *elemr=NULL;
  Datasets  *lcaser=NULL;


  if (setname[0]!=0) /* store nodes and elements in set */
  {
    if(printFlag) printf (" read %s in set:%s\n", frdfile, setname );
    readfrd( frdfile, anzr, &noder, &elemr, &lcaser, 1);
  
  
    if(anzr->n>0)
    {
      for (i=0; i<anzr->n; i++)
      {
        if ((noder[i].nr <= anz->nmax)&&(noder[i].nr >= anz->nmin))
        {
          if(setNr<0) { sprintf (datum, "%d", noder[i].nr); setNr=pre_seta( setname, "n", datum ); }
          else seta( setNr, "n", noder[i].nr );
        }
        else printf (" node %d is outside range of known nodes\n", noder[i].nr);
      }
    }
    if(anzr->e>0)
    {
      for (i=0; i<anzr->e; i++)
      {
        if ((elemr[i].nr <= anz->emax)&&(elemr[i].nr >= anz->emin))
        {
          if(setNr<0) { sprintf (datum, "%d", elemr[i].nr); pre_seta( setname, "e", datum ); }
          else seta( setNr, "e", elemr[i].nr );
        }
      else printf (" element %d is outside range of known elements\n", elemr[i].nr);
      }
    }
    free(elemr);
    free(noder);
    free(lcaser);
  }
}



long swap_long(char *ptr)
{
  long buf,buf2;
  char *pbuf, *pbuf2;

  pbuf=(char *)&buf;
  pbuf2=(char *)&buf2;

  *pbuf2=*(ptr);  pbuf2++;
  *pbuf2=*(ptr+1);  pbuf2++;
  *pbuf2=*(ptr+2);  pbuf2++;
  *pbuf2=*(ptr+3); 

  *pbuf=*(ptr+3);  pbuf++;
  *pbuf=*(ptr+2);  pbuf++;
  *pbuf=*(ptr+1);  pbuf++;
  *pbuf=*(ptr);

  // printf("in:%u out:%u\n", buf2, buf);
  return(buf);
}


#define BYTES 100000000
BGpicture *readxwd( char * datin)
{
  int i, j, k, n, fp, offset;
  int bytes_per_line, bits_per_pixel;
  XWDFileHeader *xwdheader;

  static char buf[BYTES];
  static char buf2[BYTES];
  static BGpicture bgpicture;

  char aux;

  fp = open ( datin, O_RDONLY);
  if( read( fp, buf, BYTES)<1)
  {
    printf("ERROR: File:%s could not be opened\n", datin) ;
    return(0);
  }
  xwdheader=(XWDFileHeader *)&buf[0];

  /* swap from big-endian to little-endian */
  printf("header: %d or swaped: %ld\n", xwdheader->header_size, swap_long((char *)&xwdheader->header_size));
  if ((xwdheader->header_size>300)||(xwdheader->header_size<100))
  {
    offset=swap_long((char *)&xwdheader->header_size)+swap_long((char *)&xwdheader->ncolors)*sz_XWDColor;
    bytes_per_line=swap_long((char *)&xwdheader->bytes_per_line);
    bgpicture.width=(GLsizei)swap_long((char *)&xwdheader->window_width);
    bgpicture.height=(GLsizei)swap_long((char *)&xwdheader->window_height);
    bits_per_pixel=(int)swap_long((char *)&xwdheader->bits_per_pixel);

    /*
        printf(" XWD_FILE_VERSION above 	      :%d \n", swap_long(&xwdheader->file_version ));        
        printf(" ZPixmap or XYPixmap 		      :%d \n", swap_long(&xwdheader->pixmap_format ));       
        printf(" Pixmap depth 			      :%d \n", swap_long(&xwdheader->pixmap_depth ));        
        printf(" Pixmap width 	       		       :%d\n", swap_long(&xwdheader->pixmap_width ));        
        printf(" Pixmap height 		      	      :%d \n", swap_long(&xwdheader->pixmap_height ));       
        printf(" Bitmap x offset, normally 0   	      :%d\n", swap_long(&xwdheader->xoffset ));             
        printf(" of image data: MSBFirst, LSBFirs     :%d \n", swap_long(&xwdheader->byte_order ));          
        printf(" bitmap_unit			      :%d \n", swap_long(&xwdheader->bitmap_unit ));
        printf(" bitmaps only: MSBFirst, LSBFirst     :%d\n", swap_long(&xwdheader->bitmap_bit_order ));    
        printf(" bitmap_pad			      :%d \n", swap_long(&xwdheader->bitmap_pad ));
        printf(" Bits per pixel 		      :%d \n", swap_long(&xwdheader->bits_per_pixel ));      
        printf(" bytes_per_line			      :%d \n", swap_long(&xwdheader->bytes_per_line ));
        printf(" Class of colormap 		      :%d \n", swap_long(&xwdheader->visual_class ));        
        printf(" Z red mask			      :%d \n", swap_long(&xwdheader->red_mask ));            
        printf(" Z green mask			      :%d \n", swap_long(&xwdheader->green_mask ));          
        printf(" Z blue mask 			      :%d \n", swap_long(&xwdheader->blue_mask ));           
        printf(" Log2 of distinct color values	      :%d \n", swap_long(&xwdheader->bits_per_rgb ));        
        printf(" Number of entries in colormap; not used? :%d  \n", swap_long(&xwdheader->colormap_entries ));    
        printf(" Number of XWDColor structures	       :%d\n", swap_long(&xwdheader->ncolors ));             
        printf(" Window width			      :%d \n", swap_long(&xwdheader->window_width ));        
        printf(" Window height			      :%d \n", swap_long(&xwdheader->window_height ));       
        printf(" Window upper left X coordinate	      :%d \n", swap_long(&xwdheader->window_x ));            
        printf(" Window upper left Y coordinate	      :%d \n", swap_long(&xwdheader->window_y ));            
        printf(" Window border width                  :%d \n", swap_long(&xwdheader->window_bdrwidth ));     
    */
  }
  else
  {
    offset=xwdheader->header_size + xwdheader->ncolors*sz_XWDColor;
    bytes_per_line=xwdheader->bytes_per_line;
    bgpicture.width=(GLsizei)xwdheader->window_width;
    bgpicture.height=(GLsizei)xwdheader->window_height;
    bits_per_pixel=(int)xwdheader->bits_per_pixel;

    printf("ncolors:%d\n", xwdheader->ncolors);
  }

  bgpicture.name=&buf[100];
  printf("windowName:%s\n", bgpicture.name);
  if(bits_per_pixel==32) { bgpicture.format=GL_RGBA;  bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==24) { bgpicture.format=GL_RGB;   bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==8)  { bgpicture.format=GL_RGBA;  bgpicture.type=GL_UNSIGNED_BYTE; }
  if(bits_per_pixel==1)  { bgpicture.format=GL_COLOR_INDEX;  bgpicture.type=GL_BITMAP; }


  /* inverse the lines (last is first) */
  n=0;
  for(j=0; j<bgpicture.height; j++)
  {
    for(i=0; i<bytes_per_line; i++)
    {
      buf2[n]=buf[offset-1+ (bytes_per_line*(bgpicture.height-j-1)) +i +1];
      if(bits_per_pixel==8)
      {
        for (k=1; k<4; k++) buf2[n+k]=buf2[n];
        n+=3;
      }
      n++;
    }
  }
  /* convert the image data from RGBA to BGRA if 32bit picture */
  if (bits_per_pixel == 32) for (i=0; i < bgpicture.width * bgpicture.height * bits_per_pixel/8 ; i+= bits_per_pixel/8)
  {
      aux = buf2[i];
      buf2[i] = buf2[i+2];
      buf2[i+2] = aux;
  }

  bgpicture.pixels=&buf2[0];
  printf("pixel-offset:%d\n", offset);
  printf("bits_per_pixel:%d\n", bits_per_pixel);
  printf("bytes_per_line:%d\n", bytes_per_line);
  printf("window_width:%d\n", bgpicture.width);
  printf("window_height:%d\n", bgpicture.height);
  /* printf("window-pixels:%x\n", bgpicture.pixels); */

  return(&bgpicture);
}

BGpicture *_readxwd(void)
{
   int i, j, c;
   static GLubyte checkImage[64][64][3];
   static BGpicture bgpicture;
   static char name[100];

   for (i = 0; i < 64; i++) {
      for (j = 0; j < 64; j++) {
         c = ((((i&0x8)==0)^((j&0x8)))==0)*255;
         checkImage[i][j][0] = (GLubyte) c/(i+1);
         checkImage[i][j][1] = (GLubyte) c/(j+1);
         checkImage[i][j][2] = (GLubyte) c;
      }
   }
  strcpy(name,"CheckImage");

  bgpicture.name=&name[0];
  bgpicture.width=64;
  bgpicture.height=64;
  bgpicture.format=GL_RGB;
  bgpicture.type=GL_UNSIGNED_BYTE;
  bgpicture.pixels=&checkImage;

  return(&bgpicture);
}



void pre_read( char *record )
{
  char      datin[MAX_LINE_LENGTH];
  char      param1[MAX_LINE_LENGTH];
  char      param2[MAX_LINE_LENGTH];
  char      ext[MAX_LINE_LENGTH];
  int       e,i,j,n,l,lp, length, nset, emax=0, nmax=0;
  Summen    apre[1];
  Nodes     *npre=NULL;
  Elements  *epre=NULL;
  Datasets  *lpre=NULL;
  Sets      *spre=NULL;
  int *newnode=NULL, *newelem=NULL;
  char addDispFlagLocal=0;

  param1[0]=0;
  param2[0]=0;
  length = sscanf( record, "%s %s %s", datin, param1, param2 );

  /* the filetype is determined by its extension except fbd */
  /* frd: frd-format, nodes, element-numbers will be stored in a set, but are not re-defined */
  /* inp: abaqus format, nodes, elements, sets */

  /* determine the extension */
  for(i=strlen(datin); i>=0; i--) if(datin[i]=='.') break;
  for(j=++i; j<strlen(datin); j++) ext[j-i]=datin[j]; ext[j-i]=0;

  if(param1[0]=='-')
  { readlist( datin, &param1[1] ); }
  else if(compare( ext, "xwd", 3)== 3)
  {
    bgpicture=readxwd( datin);
    if(length==2) { bgpicture->zoom[0]=atof(param1); bgpicture->zoom[1]=atof(param1); }
    else if(length>2) { bgpicture->zoom[0]=atof(param1); bgpicture->zoom[1]=atof(param2); }
    else bgpicture->zoom[0]=bgpicture->zoom[1]=1.;
  }
  else if((compare( ext, "edg", 3)== 3)||(compare( param1, "edg", 3)== 3)) 
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    apre->n=0;
    apre->e=0;
    apre->f=0;
    apre->g=0;
    apre->emax=0;  apre->emin=MAX_INTEGER;
    apre->nmax=0;  apre->nmin=MAX_INTEGER;
    apre->l=0;
    readEdges( datin, apre, &npre, &epre );
    printf("add nodes:%d maxnr:%d\n",apre->n, apre->nmax);  

    /* modify node and element numbers if param2 == add */
    if((param2[0]=='A')||(param2[0]=='a'))
    {
      renumberfrd( anz->emax+1, anz->nmax+1, &apre[0], &npre, &epre, &lpre, &newnode, &newelem );
      free(newelem);
      free(newnode);
    }

    /* create the nodes and elements or overwrite old ones */
    for(i=0; i<apre->n; i++)  nod( anz, &node, 1, npre[i].nr, npre[npre[i].nr].nx, npre[npre[i].nr].ny, npre[npre[i].nr].nz, 1 );
    for(i=0; i<apre->e; i++) elem_define( epre[i].nr, epre[i].type, epre[i].nod, 1,  0 );
    printf("elements:%d maxnr:%d\n",anz->e, anz->emax);  

    free(npre);
    free(epre);

    /* new scaling for drawing */
    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    scalPoints ( anzGeo->p, point, scale );
    scalSurfs( anzGeo->s, surf, scale);

    /* from here code from pre_mesh() */
    /* das neue netz muss noch zur beleuchteten ansicht aufbereitet werden  */
    makeSurfaces();
    getElemNormalen( e_enqire, node, anz->e );
    realloc_colNr();

    if(addDispFlagLocal==2) { addDispToCoordinates(node); }
  }
  else if((compare( ext, "frd", 3)== 3)&&((param1[0])&&compareStrings(param1,"new")>0))
  {
    pre_del("me");
    /* descale is needed since scale works only when a descale was done before */ 
    descalNodes ( anz->n, node, scale );
    iniMeshData( datin, "frd" );

    if(anz->n>0)
    {
      if ( (set[setall].node = (int *)realloc((int *)set[setall].node, (anz->n+1) * sizeof(int))) == NULL )
        printf(" ERROR: malloc failed in set[%d]:%s\n\n", 0, set[setall].name);
      set[setall].anz_n=0;
      for (i=0; i<anz->n; i++) if(!node[node[i].nr].pflag)
      {
        set[setall].node[i]= node[i].nr;
        set[setall].anz_n++;
      }
      qsort( set[setall].node, set[setall].anz_n, sizeof(int), (void *)compareInt );
    }
    if(anz->e>0)
    {
      set[setall].anz_e=anz->e;
      if ( (set[setall].elem = (int *)realloc((int *)set[setall].elem, (anz->e+1) * sizeof(int))) == NULL )
        printf(" ERROR: malloc failed in set[%d]:%s\n\n", 0, set[setall].name);
      for (i=0; i<set[setall].anz_e; i++) set[setall].elem[i]= e_enqire[i].nr;
      qsort( set[setall].elem, set[setall].anz_e, sizeof(int), (void *)compareInt );
    }

    /* new scaling for drawing */
    descalAll();
    getScaleValues( setall, set, point, node, scale);
    scalNodes ( anz->n, node, scale );
    scalPoints ( anzGeo->p, point, scale );
    scalSurfs( anzGeo->s, surf, scale);
    /* calc additional entities only if the block was not jumped during read */
    for (i=0; i<anz->olc; i++)  if (lcase[i].loaded)
      calcDatasets( i, anz, node, lcase );
    /* create the mainmenu */
    createDatasetEntries();
  }
  else if((compare( ext, "frd", 3)== 3)&&((param1[0])&&compareStrings(param1,"add")<1)&&((param1[0])&&compareStrings(param1,"ext")<1))
  {
    readfrdfile(datin,param1);
  }
  else if((compare( ext, "frd", 3)== 3)||(compare( param1, "ng", 2)== 2)||(compare( ext, "stl", 3)== 3)||(compare( ext, "inp", 3)== 3)||(compare( param1, "inp", 3)== 3)||(compare( param1, "foam", 4)== 4)) 
  {
    if(addDispFlag==1) { addDispToCoordinates(node); addDispFlagLocal=2; }
    apre->n=0;
    apre->e=0;
    apre->f=0;
    apre->g=0;
    apre->emax=0;  apre->emin=MAX_INTEGER;
    apre->nmax=0;  apre->nmin=MAX_INTEGER;
    apre->b=0;
    apre->c=0;
    apre->l=0;
    apre->sets=0;
    apre->mats=anz->mats;
    apre->amps=anz->amps;

    if(compare( ext, "frd", 3)== 3)
    {
      readfrd( datin, apre, &npre, &epre, &lpre, 1);
    }
    else if(compare( param1,"foam",4)==4)
    {
      readFoam( datin, apre, &spre, &npre, &epre, &lpre );
    }
    else if(compare( ext, "stl",3)== 3)
    { 
      readStl( datin, apre, &npre, &epre, &lpre );
    }
    else if(compare( param1,"ng",2)==2)
    {
      readNG( datin, apre, &spre, &npre, &epre, &lpre );
    }
    else
    {
      if( readccx( datin, apre, &spre, &npre, &epre, &lpre) == -1) return;
    }

    /* if 'nom(mesh)' is specified, scip the mesh-definition */
    if( (compare( param1, "nom", 3)== 3)||(compare( param2, "nom", 3)== 3))
    {
      if(anz->orign!=apre->n) printf("WARNING: Number of nodes:%d in the inp-file is different to the current number of nodes:%d. The meshes are potentially different.\n",apre->n,anz->orign ); 
      if(anz->e!=apre->e) printf("WARNING: Number of elements:%d in the inp-file is different to the current number of elements:%d. The meshes are potentially different.\n",apre->e,anz->e); 
    }
    else
    {
      /* free the additional midside-nodes for higher order elements */
      for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
      anz->n= anz->orign;
      anz->nmax=anz->orignmax;
  
      /* modify node and element numbers if param2 == add */
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3))
      {
        emax=apre->emax;
        nmax=apre->nmax;
        renumberfrd( anz->emax+1, anz->nmax+1, &apre[0], &npre, &epre, &lpre, &newelem, &newnode );
      }
  
      /* create the nodes or overwrite old ones */
      for(i=0; i<apre->n; i++) 
      {
        nod( anz, &node, 1, npre[i].nr, npre[npre[i].nr].nx, npre[npre[i].nr].ny, npre[npre[i].nr].nz, 1 );
      }
      /* new total number of mesh nodes */
      anz->orignmax = anz->nmax;
      anz->orign = anz->n;
      
      /* generate elements */
      for(i=0; i<apre->e; i++)
      {
        e=elem_define( epre[i].nr, epre[i].type, epre[i].nod, 1, epre[i].attr );
        e_enqire[epre[i].nr].mat=epre[i].mat;
      }
  
      /* new scaling for drawing */
      descalAll();
      getScaleValues( setall, set, point, node, scale);
      scalNodes ( anz->n, node, scale );
      scalPoints ( anzGeo->p, point, scale );
      scalSurfs( anzGeo->s, surf, scale);
  
      /* new midnodes for drawing */
      adjustDrawNodes(1);

      getElemNormalen( e_enqire, node, anz->e );
      makeSurfaces();
    }

    /* add the new datasets if no old exist */
    if(anz->l==0)
    {
      lcase=lpre;
      anz->olc=anz->l=apre->l;
      lpre=NULL;
      for(l=0; l<anz->l; l++)
      {
        calcDatasets( l, anz, node, lcase );
        recompileEntitiesInMenu(l);
      }
    }
    else
    {
      /* add the new nodes to the existing datasets if option ext or add was provided, else add datasets to the existing ones */
      if( (compare( param1, "add", 3)== 3)||(compare( param2, "add", 3)== 3)||(compare( param1, "ext", 3)== 3)||(compare( param2, "ext", 3)== 3))
      {
        lp=0;
        for(l=0; l<anz->l; l++)
        {
          /* fill the datasets */
          if((lp<apre->l)&&(lcase[l].ncomps>=lpre[lp].ncomps)&&(compareStrings(lcase[l].name, lpre[lp].name)>0))
          {
            /* check if the data of the specified lcase (Dataset) are already available */
            if (!lcase[l].loaded)
            {
              if( pre_readfrdblock(copiedNodeSets , l, anz, node, lcase )==-1) 
              {
                printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", l+1); 
                return;
              }
              for(i=0; i<lcase[l].ncomps; i++)
              {
                if ( (lcase[l].dat[i] = (float *)realloc(lcase[l].dat[i], (anz->nmax+1) * sizeof(float))) == NULL )
                printf("\n\n ERROR: realloc failure nod\n\n" );
              }
              for(n=0; n<apre->n; n++) 
              {
                for(i=0; i<lpre[lp].ncomps; i++)
                {
                  lcase[l].dat[i][npre[n].nr]=  lpre[lp].dat[i][npre[n].nr];
                  if (lcase[l].dat[i][npre[n].nr] >  lcase[l].max[i])
                  {  lcase[l].max[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmax[i]=npre[n].nr; }
                  if (lcase[l].dat[i][npre[n].nr] <  lcase[l].min[i])
                  {  lcase[l].min[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmin[i]=npre[n].nr; }
                }     
              }     
              calcDatasets( l, anz, node, lcase );
              recompileEntitiesInMenu(l);
            }
            else
            {
              for(n=0; n<apre->n; n++) 
              {
                for(i=0; i<lpre[lp].ncomps; i++)
                {
                  lcase[l].dat[i][npre[n].nr]=  lpre[lp].dat[i][npre[n].nr];
                  if (lcase[l].dat[i][npre[n].nr] >  lcase[l].max[i])
                  {  lcase[l].max[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmax[i]=npre[n].nr; }
                  if (lcase[l].dat[i][npre[n].nr] <  lcase[l].min[i])
                  {  lcase[l].min[i]=lcase[l].dat[i][npre[n].nr]; lcase[l].nmin[i]=npre[n].nr; }
                }     
              }     
              calcDatasets( l, anz, node, lcase );
            }
  	    printf(" Add dataset[%d]:%s to existing dataset[%d]:%s\n",lp+1, lpre[lp].name, l+1,lcase[l].name);
            lp++;     
          }
  	  else
          {
  	    if(lp) printf(" WARNING: data from new model have different nr of components for dataset[%d]:%s and could not be merged.\n",l+1,lcase[l].name);
            if( apre->l > anz->l ) { lp++; l--; }
            if(( lp >= apre->l)||(l<0))  break;
  	  }
        }
      }
      /* add the new datasets */
      else
      {
        printf("additional Datasets:%d\n",apre->l);  
        if( ( lcase=(Datasets *)realloc((Datasets *)lcase,(anz->l+apre->l+1)*sizeof(Datasets))  )==NULL)
        printf("\n\n ERROR: realloc failed\n\n") ;
        for(l=0; l<apre->l; l++)
	{
          lcase[anz->l+l]=lpre[l];
          if (lcase[anz->l+l].loaded) calcDatasets( anz->l+l, anz, node, lcase );
          if(activWindow!=-1) recompileEntitiesInMenu(anz->l+l);
	}
        anz->l+=apre->l;
        anz->olc=anz->l;
      }
    }
    if(activWindow!=-1) createDatasetEntries();

    /* add materials and amplitudes */
    anz->mats+=apre->mats;
    anz->amps+=apre->amps;

    /* generate sets */
    for(i=0; i<apre->sets; i++)
    {
      nset=getSetNr(spre[i].name);
      if(nset<0) nset=pre_seta(spre[i].name,"i",0);
      set[nset].type=spre[i].type;
      set[nset].material=spre[i].material;
      if(newnode) for (j=0; j<spre[i].anz_n; j++) { if(spre[i].node[j]<=nmax) seta(nset,"n",newnode[spre[i].node[j]]); }
      else for (j=0; j<spre[i].anz_n; j++) seta(nset,"n",spre[i].node[j]);
      if(newelem) for (j=0; j<spre[i].anz_e; j++) { if(spre[i].elem[j]<=emax) seta(nset,"e",newelem[spre[i].elem[j]]); }
      else for (j=0; j<spre[i].anz_e; j++) seta(nset,"e",spre[i].elem[j]);

      for (j=0; j<spre[i].anz_elf; j++)
      {
        if(newelem) e=newelem[spre[i].elf[j].e]; else e=spre[i].elf[j].e;

        /* for the moment a negative face-orientation can not be stored and is changed to positive */
        if((e_enqire[e].type>6)&&(e_enqire[e].type<11))
        {
          if(e_enqire[e].attr>3)
          {
            spre[i].elf[j].f++;
            if(e_enqire[e].type<9)
	    {
              if(spre[i].elf[j].f>4) spre[i].elf[j].f=1;
	    }
	    else
	    {
              if(spre[i].elf[j].f>5) spre[i].elf[j].f=1;
	    }
	  }
          else
          {
            spre[i].elf[j].f--;
            if(spre[i].elf[j].f==0) spre[i].elf[j].f=1;
	  }
	}

        /* store the actual face-indexes in the referenced sets */
        seta( nset, "f", face[e].indx[spre[i].elf[j].f]);
      }  
    }

    /* delete elements in special sets created by readNG */
    // (Peter)  unless keyword ndsb  (NoDeleteShellsBeams) is present
    if((compare(param1,"ng",2)==2) && (compare(param2, "ndsb",4)!=4))
    {
      for (i=0; i<anz->sets; i++)
      {
        if(set[i].name!=NULL)
	{
          if(compare(set[i].name, "+typ3", 5)==5) break;
          if(compare(set[i].name, "+typ6", 5)==5) break;
	}
      }
      if(i<anz->sets)
      {
       printf("found volume elements -> delete all shells and beams.\n");
       for (i=0; i<anz->sets; i++)
       {
        if(set[i].name!=NULL)
	{
          if(compare(set[i].name, "+typ7", 5)==5) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ8", 5)==5) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ11", 6)==6) delElem( set[i].anz_e, set[i].elem );
          if(compare(set[i].name, "+typ12", 6)==6) delElem( set[i].anz_e, set[i].elem );
	}
       }
       for (i=0; i<anz->sets; i++)
       {
        if((set[i].name!=NULL)&&(compare(set[i].name, "+set", 4)==4))
        {
          /* get the nodes and faces */
          completeSet(set[i].name, "do");
        }
       }
      }
    }

    realloc_colNr();

    free(newelem);
    free(newnode);

    for(j=0; j<apre->sets; j++) {  delSetx(spre[j].name); }
    free(spre); spre=NULL;
    free(npre); npre=NULL;
    free(epre); epre=NULL;
    free(lpre); lpre=NULL;
 
    /* calculate the line-shapes */
    for (i=0; i<anzGeo->l; i++) repLine(i);
    /* calculate the nurbl-controll-points */
    for (i=0; i<anzGeo->nurl; i++) repNurl(i);
    /* calculate the nurbs-controll-points */
    for (i=0; i<anzGeo->nurs; i++) repNurs(i);

    if(addDispFlagLocal==2) { addDispToCoordinates(node); }
  }
  else if ((length==1)||(compare( param1, "rep", 3)== 3))
  {
    readfbd( datin, 0 );
    /* updateDispLists would overwrite the elem and model edges of the 1st frame */
    //if(!animFlag) updateDispLists();
  }
  else if (compare( param1, "add", 3)== 3)
  {
    readfbd( datin, 1 );
    //if(!animFlag) updateDispLists();
  } 
  else printf("ERROR, no matching file-type found\n");

  updateDispLists(); 
  createSuperSets(); 

  printf (" ready\n");
}


/* combine single sets to a superSet */
void createSuperSets(void)
{
  int i,j,s,flmSet=-1, radSet=-1, cflxSet=-1, dflxSet=-1;

  s=anz->sets;
  for (i=0; i<s; i++) if(set[i].name!=(char *)NULL)
  {
    /* create additional special sets which add sets of a ceratin type */
    if(compare(set[i].name,"+flm",4)==4)
    {
      if(flmSet<0) flmSet=pre_seta("+flm","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(flmSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+rad",4)==4)
    {
      if(radSet<0) radSet=pre_seta("+rad","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(radSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+cflx",5)==5)
    {
      if(cflxSet<0) cflxSet=pre_seta("+cflx","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(cflxSet,"f",set[i].face[j]);
    }
    if(compare(set[i].name,"+dflx",5)==5)
    {
      if(dflxSet<0) dflxSet=pre_seta("+dflx","i",0);
      for (j=0; j<set[i].anz_f; j++) seta(dflxSet,"f",set[i].face[j]);
    }
  }
}



/* define or change the element-typ in the struct of the lines, surfs, bodies */
void pre_elty( char *record )
{
  int i, length, setNr;
  char      setname[MAX_LINE_LENGTH], etype[MAX_LINE_LENGTH], eattr[MAX_LINE_LENGTH], eparm[MAX_LINE_LENGTH];

  setname[0]=etype[0]=eparm[0]='\0';
  length= sscanf( record, "%s%s%s", setname, etype, eparm);
  if(compareStrings(setname, "ALL")==3) strcpy(setname,"all");

  if(length<1)
  {
    /* show all sets which have an element type assigned */
    for (setNr=0; setNr<anz->sets; setNr++)
    {
      if((set[setNr].etyp!=0)&&(set[setNr].name!=(char *)NULL))
      {
        if (set[setNr].etyp < 0) strcpy( etype, "unknown type ");
        if (set[setNr].etyp == 1)  strcpy( etype, "he8");
        if (set[setNr].etyp == 2)  strcpy( etype, "pe6"); 
        if (set[setNr].etyp == 3)  strcpy( etype, "te4"); 
        if (set[setNr].etyp == 4)  strcpy( etype, "he20"); 
        if (set[setNr].etyp == 5)  strcpy( etype, "pe15");
        if (set[setNr].etyp == 6)  strcpy( etype, "te10"); 
        if (set[setNr].etyp == 7)  strcpy( etype, "tr3"); 
        if (set[setNr].etyp == 8)  strcpy( etype, "tr6"); 
        if (set[setNr].etyp == 9)  strcpy( etype, "qu4"); 
        if (set[setNr].etyp == 10) strcpy( etype, "qu8"); 
        if (set[setNr].etyp == 11) strcpy( etype, "be2"); 
        if (set[setNr].etyp == 12) strcpy( etype, "be3");
        if (set[setNr].etyp > 12) strcpy( etype, "unknown type ");

        if (set[setNr].eattr < -2) strcpy( eattr, " unknown attr");
        if (set[setNr].eattr == -2) strcpy( eattr, "g (glu mesh)");
        if (set[setNr].eattr == -1) strcpy( eattr, "u (unstructured)");
        if (set[setNr].eattr == 0) strcpy( eattr, " ");
        if (set[setNr].eattr == 1) strcpy( eattr, "r");
        if (set[setNr].eattr == 2) strcpy( eattr, "i");
        if (set[setNr].eattr == 3) strcpy( eattr, "d");
        if (set[setNr].eattr == 4) strcpy( eattr, "e");
        if (set[setNr].eattr == 5) strcpy( eattr, "s");
        if (set[setNr].eattr == 6) strcpy( eattr, "c");
        if (set[setNr].eattr == 7) strcpy( eattr, "f");
        if (set[setNr].eattr > 7) strcpy( eattr, " unknown attr");

        if (set[setNr].eparm != (char *)NULL) strcpy( eparm, set[setNr].eparm);
        else eparm[0]=0;

        printf("set:%s element-type: %s%s %s\n", set[setNr].name, etype, eattr, eparm);
      }
    }
  }
  else if(length==1)
  {
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR in pre_elty: set:%s does not exist\n", setname);
      return;
    }

    /* delete all element definitions if the setname is "all" */
    if(compareStrings(setname, "all")>0)
    {
      for (setNr=0; setNr<anz->sets; setNr++)
      {
        set[setNr].etyp=0;
        set[setNr].eattr=0;
        if(set[setNr].eparm!=(char *)NULL)
	{
          free(set[setNr].eparm);
          set[setNr].eparm=(char *)NULL;
	}
        for (i=0; i<set[setNr].anz_b; i++)
        {
          body[set[setNr].body[i]].eattr=set[setNr].eattr ;
          body[set[setNr].body[i]].etyp=set[setNr].etyp ;
          if(body[set[setNr].body[i]].eparm!=(char *)NULL)
	  { free(body[set[setNr].body[i]].eparm) ; body[set[setNr].body[i]].eparm=(char *)NULL; }
        }
        for (i=0; i<set[setNr].anz_s; i++)
        {
          surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
          surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
          if(surf[set[setNr].surf[i]].eparm!=(char *)NULL)
	  { free(surf[set[setNr].surf[i]].eparm) ; surf[set[setNr].surf[i]].eparm=(char *)NULL; }
        }
        for (i=0; i<set[setNr].anz_l; i++)
        {
          line[set[setNr].line[i]].eattr=set[setNr].eattr ;
          line[set[setNr].line[i]].etyp=set[setNr].etyp ;
        }
      }
    }
    /* delete only the element-def of this set */
    else
    {
      set[setNr].etyp=0;
      set[setNr].eattr=0;
      set[setNr].eparm=(char *)NULL;
      if(set[setNr].eparm!=(char *)NULL)
      {
        free(set[setNr].eparm);
        set[setNr].eparm=(char *)NULL;
      }
      for (i=0; i<set[setNr].anz_b; i++)
      {
        body[set[setNr].body[i]].eattr=set[setNr].eattr ;
        body[set[setNr].body[i]].etyp=set[setNr].etyp ;
        if(body[set[setNr].body[i]].eparm!=(char *)NULL)
        { free(body[set[setNr].body[i]].eparm) ; body[set[setNr].body[i]].eparm=(char *)NULL; }
      }
      for (i=0; i<set[setNr].anz_s; i++)
      {
        surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
        surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
        if(surf[set[setNr].surf[i]].eparm!=(char *)NULL)
        { free(surf[set[setNr].surf[i]].eparm) ; surf[set[setNr].surf[i]].eparm=(char *)NULL; }
      }
      for (i=0; i<set[setNr].anz_l; i++)
      {
        line[set[setNr].line[i]].eattr=set[setNr].eattr ;
        line[set[setNr].line[i]].etyp=set[setNr].etyp ;
      }
    }
  }
  else
  {
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR in pre_elty: set:%s does not exist\n", setname);
      return;
    }

    for(i=0;i<strlen(etype); i++) etype[i]=toupper(etype[i]);
    for(i=0;i<strlen(eparm); i++) eparm[i]=toupper(eparm[i]);
    if(strlen(eparm))
    {
      if((set[setNr].eparm= (char *)realloc((char *)set[setNr].eparm, (strlen(eparm)+1)*sizeof(char))) == NULL )
      { printf("ERROR: malloc failed in seta()\n\n" ); return; }
      strcpy(set[setNr].eparm, eparm);
    }
    else { free(set[setNr].eparm); set[setNr].eparm=(char *)NULL; } 

    if( compare( etype, "BE2", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=11; set[setNr].eattr=0; }
      else if(etype[3]=='D') { set[setNr].etyp=11; set[setNr].eattr=3; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "BE3", 3) == 3)
    {
      set[setNr].etyp=12; set[setNr].eattr=0;
    }
    else if(compare( etype, "TR3", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=7; set[setNr].eattr=0; }
      else if(etype[3]=='U') { set[setNr].etyp=7; set[setNr].eattr=-1; }
      else if(etype[3]=='G') { set[setNr].etyp=7; set[setNr].eattr=-2; }
      else if(etype[3]=='E') { set[setNr].etyp=7; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=7; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=7; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TR6", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=8; set[setNr].eattr=0; }
      else if(etype[3]=='U') { set[setNr].etyp=8; set[setNr].eattr=-1; }
      else if(etype[3]=='E') { set[setNr].etyp=8; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=8; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=8; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "QU4", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=9; set[setNr].eattr=0; }
      else if(etype[3]=='E') { set[setNr].etyp=9; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=9; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=9; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "QU8", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=10; set[setNr].eattr=0; }
      else if(etype[3]=='E') { set[setNr].etyp=10; set[setNr].eattr=4; }
      else if(etype[3]=='S') { set[setNr].etyp=10; set[setNr].eattr=5; }
      else if(etype[3]=='C') { set[setNr].etyp=10; set[setNr].eattr=6; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "HE8", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=1; set[setNr].eattr=0; }
      else if(etype[3]=='R') { set[setNr].etyp=1; set[setNr].eattr=1; }
      else if(etype[3]=='I') { set[setNr].etyp=1; set[setNr].eattr=2; }
      else if(etype[3]=='F') { set[setNr].etyp=1; set[setNr].eattr=7; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "HE20", 4) == 4)
    {
      if(strlen(etype)==4)   { set[setNr].etyp=4; set[setNr].eattr=0; }
      else if(etype[4]=='R') { set[setNr].etyp=4; set[setNr].eattr=1; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TE4", 3) == 3)
    {
      if(strlen(etype)==3)   { set[setNr].etyp=3; set[setNr].eattr=0; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else if(compare( etype, "TE10", 4) == 4)
    {
      if(strlen(etype)==4)   { set[setNr].etyp=6; set[setNr].eattr=0; }
      else if(etype[4]=='M') { set[setNr].etyp=6; set[setNr].eattr=8; }
      else printf("ERROR: type %s not known\n", etype);
    }
    else 
    {
      printf("ERROR: element-type:%s not known (choose either be2,be3,tr3,tr6,qu4,qu8,he8,he20,te4,te10)\n",etype);
      return;
    }
    
    /* assign the element-type to the appropriate entities */ 
    /* assign the attribute */
    /* get the basis formulation of the etyp */
    if((set[setNr].etyp==1)||(set[setNr].etyp==3)||(set[setNr].etyp==4)||(set[setNr].etyp==6))
    {
      for (i=0; i<set[setNr].anz_b; i++)
      {
        body[set[setNr].body[i]].eattr=set[setNr].eattr ;
        body[set[setNr].body[i]].etyp=set[setNr].etyp ;
        if(set[setNr].eparm!=(char *)NULL) {
        if((body[set[setNr].body[i]].eparm= (char *)realloc((char *)body[set[setNr].body[i]].eparm, (strlen(set[setNr].eparm)+1)*sizeof(char))) == NULL )
        { printf("ERROR: malloc failed in prnt()\n\n" ); return; }
        strcpy(body[set[setNr].body[i]].eparm,set[setNr].eparm); }
      }
    }
    if((set[setNr].etyp>=7)&&(set[setNr].etyp<=10))
    {
      for (i=0; i<set[setNr].anz_s; i++)
      {
        surf[set[setNr].surf[i]].eattr=set[setNr].eattr ;
        surf[set[setNr].surf[i]].etyp=set[setNr].etyp ;
        if(set[setNr].eparm!=(char *)NULL)
        {
          if((surf[set[setNr].surf[i]].eparm= (char *)realloc((char *)surf[set[setNr].surf[i]].eparm, (strlen(set[setNr].eparm)+1)*sizeof(char))) == NULL )
          { printf("ERROR: malloc failed in prnt()\n\n" ); return; }
          strcpy(surf[set[setNr].surf[i]].eparm,set[setNr].eparm);
        }
        else { free(surf[set[setNr].surf[i]].eparm); surf[set[setNr].surf[i]].eparm=(char *)NULL; }
      }
    }
    if((set[setNr].etyp>=11)&&(set[setNr].etyp<=12))
    {
      for (i=0; i<set[setNr].anz_l; i++)
      {
        line[set[setNr].line[i]].eattr=set[setNr].eattr ;
        line[set[setNr].line[i]].etyp=set[setNr].etyp ;
      }
    }
    for (i=0; i<set[setNr].anz_e; i++)
    {
      e_enqire[set[setNr].elem[i]].attr=set[setNr].eattr ;
    }
  }
}



/* define the threshold-values for element criterions */
void pre_eqal( char *record )
{
  int j;
  char  etest[MAX_LINE_LENGTH];
  double evalue;

  etest[0]='\0';
  sscanf( record, "%s%lf", etest, &evalue);
  for(j=0;j<=strlen(etest); j++) etest[j]=toupper(etest[j]);

  if(compareStrings(etest,"JBIR")>0)
  {
    eqal.jbir=evalue;
  }
  else if(compareStrings(etest,"ASPR")>0)
  {
    eqal.aspr=evalue;
  }
  else if(compareStrings(etest,"MCA")>0)
  {
    eqal.mca=evalue;
  }
  else
  {
    printf(" current element quality thresholds (0==off):\n JBIR:%lf\n ASPR:%lf\n MCA:%lf\n", eqal.jbir, eqal.aspr, eqal.mca);
  }
}


/* store the entities to be meshed in a special set and add lower entities, then mesh   */

void pre_mesh( char *record )
{
  int b,c,l,n,s,i,j,k,se, tetSet=0;
  char buffer[MAX_LINE_LENGTH],setname[MAX_LINE_LENGTH], code[4][MAX_LINE_LENGTH];
  int   anz_nmax,length, setNr, sets, meshoptFlag_length=MESHOPT_LENGTH, meshoptFlag_angle=MESHOPT_ANGLE, blockFlag=0, lonlyFlag=0, projFlag=1, tetFlag=0;
  int *openSets=NULL, sum_openSets=0;
  double teth=1e6;
  int ntmp,etmp,eSet;

  length = sscanf( record,"%s %s %s %s %s", setname,code[0],code[1],code[2],code[3] );

  for(i=0; i<length-1; i++)
  {
    if(compare(code[i],"nolength",4)==4)  meshoptFlag_length=0;
    else if(compare(code[i],"noangle",4)==4)  meshoptFlag_angle=0;
    else if(compare(code[i],"length",4)==4)  meshoptFlag_length=1;
    else if(compare(code[i],"angle",4)==4)  meshoptFlag_angle=1;
    else if(compare(code[i],"block",3)==3) blockFlag=1;
    else if(compare(code[i],"fast",3)==3) projFlag=0;
    else if(compare(code[i],"lonly",3)==3) lonlyFlag=1;
    /* a rudimentary implementation of netgen to generate tets */
    else if(compare(code[i],"tet",3)==3)
    {
      tetFlag=1;
      if(length == i+3) teth=(double)atof(code[i+1]);
    }
  }

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in pre_mesh: set:%s does not exist\n", setname);
    return;
  }
  else
    if(printFlag) printf (" set[%d]:%s will be meshed\n", setNr, setname);

  printf (" please wait for 'ready'\n");

  /* cycle through all entities and add them to the special set  */
  
  if( (se=pre_seta( specialset->mesh, "i", 0)) <0 ) return;
  if(setNr==setall)
  {
    for (i=0; i<anz->n; i++) seta( se, "n", node[i].nr );
    for (i=0; i<anz->e; i++) seta( se, "e", e_enqire[i].nr );
    for (i=0; i<anzGeo->l; i++) if( line[i].name != (char *)NULL ) seta( se, "l", i );
    for (i=0; i<anzGeo->p; i++) if( point[i].name != (char *)NULL ) seta( se, "p", i );
    for (i=0; i<anzGeo->l; i++) if( line[i].name != (char *)NULL ) seta( se, "l", i );
    for (i=0; i<anzGeo->s; i++) if( surf[i].name != (char *)NULL ) seta( se, "s", i );
    for (i=0; i<anzGeo->b; i++) if( body[i].name != (char *)NULL ) seta( se, "b", i );
  }
  else 
  {
    for (i=0; i<set[setNr].anz_n; i++) seta( se, "n", set[setNr].node[i] );
    for (i=0; i<set[setNr].anz_e; i++) seta( se, "e", set[setNr].elem[i] );
    for (i=0; i<set[setNr].anz_b; i++) seta( se, "b", set[setNr].body[i] );
    for (i=0; i<set[setNr].anz_s; i++) seta( se, "s", set[setNr].surf[i] );
    for (i=0; i<set[setNr].anz_c; i++) seta( se, "c", set[setNr].lcmb[i] );
    for (i=0; i<set[setNr].anz_l; i++) seta( se, "l", set[setNr].line[i] );
    /* second cycle through all entities and add lower ones  to the special set  */
    /* completeSet( specialset->mesh, "do") ; */

    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( se, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs and nurbs */
    for (i=0; i<set[se].anz_s; i++)
    {
      s= set[se].surf[i];
      //if(surf[s].sh>-1) seta( se, "sh", surf[s].sh );
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( se, "l", l );
        }
        else
        {
          seta( se, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[se].anz_c; i++)
    {
      c= set[se].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( se, "l", l );
      }
    }
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[se].anz_l; i++)
    {
      l= set[se].line[i];
      seta( se, "p", line[l].p1 );
      seta( se, "p", line[l].p2 );
      if (line[l].typ=='a') seta( se, "p", line[l].trk );
      if (line[l].typ=='s') seta( se, "se", line[l].trk );
    }
    /* cyrcle through all nurbs and add all points */
    /*
    for (i=0; i<set[se].anz_nurs; i++)
    {
      s= set[se].nurs[i];
      for (j=0; j<nurbs[s].u_npnt; j++)
        for (k=0; k<nurbs[s].v_npnt; k++)
          seta( se, "p", nurbs[s].ctlpnt[j][k] );
    }
    */
  }

  /* cyrcle through all bodys and check the body-etyp, for tet-requests set the surface-etyp */
  for (i=0; i<set[se].anz_b; i++)
  {
    c= set[se].body[i];
    if(body[c].etyp==3)
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        if(surf[l].etyp!=7) { surf[l].etyp=7; surf[l].eattr=-1; }
      }
    }
    if(body[c].etyp==6)
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        if(surf[l].etyp!=8) { surf[l].etyp=8; surf[l].eattr=-1; }
      }
    }
  }

  /* cyrcle again through all bodys and check the body-etyp for hex-requests, then eventually change the eattr of the surfaces */
  for (i=0; i<set[se].anz_b; i++)
  {
    c= set[se].body[i];
    if((body[c].etyp==1)||(body[c].etyp==4))
    {
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        surf[l].eattr=0;
      }
    }
  }
  
  orientSet( specialset->mesh );

  /* close all open sets, exept set[].name=='all' */
  /* necessary, if 5 or 7 sided bodies exist, otherwhise temporary geom will be assigned to them */
  j=1;
  for(i=0; i<anz->sets; i++)
  {
    if((set[i].flag=='o')&&(i!=setall))
    {
      if ( (openSets = (int *)realloc( (int *)openSets,(j+1) * sizeof(int))) == NULL )
         printf("\n\n ERROR: realloc failed in pre_mesh() openSets\n\n") ;
      openSets[j]=i;
      set[i].flag='c';
      j++;
    }
  }
  sum_openSets=j;

  /* free the additional midside-nodes for higher order elements */
  for(i=anz->orign; i<anz->n; i++) node[node[i].nr].pflag=-1;
  anz->n= anz->orign;
  anz->nmax=anz->orignmax;

  anz_nmax=anz->nmax;
  /* mesh an existing surface-mesh */
  if(tetFlag)
  {
    if(set[setNr].flag!='o') { i=1; seto(set[setNr].name); } else { i=0; }
    generateTetFromSet(se, teth);
    if(i) setc(set[setNr].name);
    anz->orignmax = anz->nmax;
    anz->orign = anz->n;
  }
  else
  {
    /* mesh the geometry */
    meshSet( specialset->mesh, blockFlag, lonlyFlag, projFlag, meshoptFlag_length, meshoptFlag_angle); 

    /* improve bad tr3 elements */
    improveBadTr3(specialset->mesh);

    /* cyrcle again through all bodys and check if mpcs have to be generated to glue incompatible element-formulations (he8+he20, tet+he20, etc) */
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      if(body[c].etyp==4)
      {
        for (j=0; j<body[c].ns; j++)
        {
          l=body[c].s[j];
  
          /* if surf[l].etyp is defined then a connection of a mid-face node and the edges has to be prepared (mpc) */
          if(((surf[l].etyp==7)||(surf[l].etyp==8))&&(body[c].etyp==4))
  	  {
            /* generate a set with all nodes and related volume elements of the surface */
            ntmp=pre_seta("-ntmp","s",surf[l].name) ;
            etmp=pre_seta("-etmp","i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(etmp,"n",set[ntmp].node[k]);
            completeSet("-etmp","up");
            eSet=pre_seta("-eSet","i",0) ;
            for(k=0; k<set[etmp].anz_e; k++) if(e_enqire[set[etmp].elem[k]].type == 4) seta(eSet,"e",set[etmp].elem[k]);
  	    completeSet("-eSet","do");
            completeSet("-ntmp","do");
  
            /* create dep and indepsets */
            if ( (depSet = (int *)realloc( depSet, (sum_equSets+1) * sizeof(int))) == NULL )
              printf("ERROR: realloc failed: meshSet\n\n" );
            if ( (indSet = (int *)realloc( indSet, (sum_equSets+1) * sizeof(int))) == NULL )
              printf("ERROR: realloc failed: meshSet\n\n" );
            sprintf(buffer,"-depSet_tmp%d", sum_equSets);
            depSet[sum_equSets]=pre_seta(buffer,"i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(depSet[sum_equSets],"n",set[ntmp].node[k]);
            for(k=0; k<set[eSet].anz_n; k++)  setr(depSet[sum_equSets],"n",set[eSet].node[k]);
            sprintf(buffer,"-indSet_tmp%d", sum_equSets);
            indSet[sum_equSets]=pre_seta(buffer,"i",0) ;
            for(k=0; k<set[ntmp].anz_n; k++)  seta(indSet[sum_equSets],"n",set[ntmp].node[k]);
            for(k=0; k<set[depSet[sum_equSets]].anz_n; k++)  setr(indSet[sum_equSets],"n",set[depSet[sum_equSets]].node[k]);
            for(k=0; k<set[eSet].anz_e; k++)  seta(indSet[sum_equSets],"e",set[eSet].elem[k]);
            sum_equSets++;

            /* delete the temporary sets */
            delSet(set[ntmp].name);
            delSet(set[etmp].name);
            delSet(set[eSet].name);
 	  }
        }
      }
    }

    /* tet-mesh */
    etmp=0;
    tetSet=-1;
    for (i=0; i<set[se].anz_b; i++)
    {
      c= set[se].body[i];
      if((body[c].etyp==3)||(body[c].etyp==6))
      {
        if(tetSet==-1)
	{
          delSet( "+tetSet" );
          tetSet=pre_seta("+tetSet","i",0);
	}

        /* the final orientation of nurbs-surfaces for the 2D-mesh requires a call to orientBody() */
        orientBody(c);

        /* get the surface-elements for later deletion */
        for(j=0; j<body[c].ns; j++)
	{
          s=body[c].s[j];
          for(k=0; k<surf[s].ne; k++) seta(tetSet,"e",surf[s].elem[k]);
	}

	printf("\n Tet-mesh body:%s\n\n", body[c].name);
        if(body[c].eparm!=(char *)NULL) etmp=generateTetFromBody(c, (double)atof(body[c].eparm));
        else etmp=generateTetFromBody(c, (double)1.e6);

        if(etmp>0)
	{
	  printf("\n %d Tet-elems in body:%s\n\n", body[c].ne, body[c].name);
 
          /* add the inner nodes and elements to the body */
          for(j=0; j<anz->sets; j++)
          {
            for (b=0; b<set[j].anz_b; b++)
            {
              if(c==set[j].body[b])
  	      {
                for(n=0; n<body[c].nn; n++) seta(j,"n", body[c].nod[n]);
                for(n=0; n<body[c].ne; n++) seta(j,"e", body[c].elem[n]);
  	      }
  	    }
          }
	}
        else
	{
          printf(" ERROR: No tet-mesh could be created for body:%s\n ", body[c].name);
          printf(" The temporary surface mesh is not deleted and can be used for debugging.\n");
          printf(" The files nodnr_ng_cgx.out elemnr_ng_cgx.out provide the link between ng and cgx entity numbers.\n");
          break;
	}
      }
    }
    /* delete the surface-elements which were used to create tets */
    if(etmp>0) zap( "+tetSet" );
  }

  /* mark the closed sets as open again */
  for(i=1; i<sum_openSets; i++) set[openSets[i]].flag='o';
  free(openSets);

  /* in case that datasets had already existed then this fields have to be extended */
  if (anz->l > 0)
  {
    for (l=0; l< anz->l; l++)
    {
      if (!lcase[l].loaded)
      {
        if( pre_readfrdblock(copiedNodeSets , l, anz, node, lcase )==-1) 
        {
          printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", l+1); 
          return;
        }
        calcDatasets( l, anz, node, lcase );
        recompileEntitiesInMenu(l);
      }
      for (i=0; i<lcase[l].ncomps; i++)
      {
        if ( (lcase[l].dat[i] = (float *)realloc( lcase[l].dat[i], (anz->nmax+2) * sizeof(float))) == NULL )
          printf("\n\n ERROR: realloc failure\n\n" );	               
        for(j=anz_nmax+1; j<=anz->nmax; j++) lcase[l].dat[i][j]=0.;
      }
    }
  }
  //anz->olc       = anz->l;

  if (calcBadElements("all")>0)
  {
    printf(" try to fix bad elements\n");
    fixMidsideNodes( specialset->njby, "lin");
    calcBadElements("all");
  }

  makeSurfaces();
  getElemNormalen( e_enqire, node, anz->e );
  realloc_colNr();

  /* add the faces */
  for (sets=0; sets<anz->sets; sets++)
  {
    if ((set[sets].type==0)&&(set[sets].name!=(char *)NULL))
    { 
      completeSet( set[sets].name, "f" );
    }
  }

  updateDispLists(); 

  /* clear special set  */
  delSet(specialset->mesh ); 
  printf (" ready\n");
}


int completeSet_Mesh( int setNr, int setNrbuf, int *elUsed, int flag)
{
  int i,j;
  int m, n, ipuf=0 ;

  if(!flag)
  {
   if (set[setNr].anz_n>0)
   {
    /* suche abhaengige elemente */
    m=set[setNrbuf].anz_e;
    for ( i=0; i<anz->e; i++)
    {
      switch(e_enqire[e_enqire[i].nr].type)
      {
        case 1:
          ipuf = 8;       /* HEXA8 */
        break;
        case 2:
          ipuf = 6;  /* PENTA6 */
        break;
        case 3:
          ipuf = 4;  /* TET4 */
        break;
        case 4:
          ipuf = 20; /* HEXA20 */
        break;
        case 5:
          ipuf = 15; /* PENTA15 */
        break;
        case 6:
          ipuf = 10; /* TET10 */
        break;
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
          ipuf = 6;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          ipuf = 8; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          ipuf = 3; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
        if(ifind(&set[setNr].node, set[setNr].anz_n, e_enqire[e_enqire[i].nr].nod[j])>-1)
        {
          //printf("    e:%d \n", e_enqire[i].nr);
          if( !elUsed[e_enqire[i].nr] )
          {
            //printf("add e:%d \n", e_enqire[i].nr);
            //seta( setNrbuf, "e", e_enqire[i].nr );
            if ( (set[setNrbuf].elem = (int *)realloc((int *)set[setNrbuf].elem, (set[setNrbuf].anz_e+1) * sizeof(int))) == NULL )
              printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNrbuf].name);
            set[setNrbuf].elem[set[setNrbuf].anz_e]= e_enqire[i].nr;
            set[setNrbuf].anz_e++;
          }
          break;
        }
      }
    }
    if(set[setNrbuf].anz_e-m)
    {
      qsort( set[setNrbuf].elem, set[setNrbuf].anz_e, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNrbuf].anz_e; i++)
      {
	if(set[setNrbuf].elem[n]!=set[setNrbuf].elem[i]) set[setNrbuf].elem[++n]=set[setNrbuf].elem[i];
      }
      set[setNrbuf].anz_e=n+1;
    }

   }
  }
  else
  {
    /* circle through all elements and add all nodes */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      switch(e_enqire[set[setNr].elem[i]].type)
      {
        case 1:
          ipuf = 8;       /* HEXA8 */
        break;
        case 2:
          ipuf = 6;  /* PENTA6 */
        break;
        case 3:
          ipuf = 4;  /* TET4 */
        break;
        case 4:
          ipuf = 20; /* HEXA20 */
        break;
        case 5:
          ipuf = 15; /* PENTA15 */
        break;
        case 6:
          ipuf = 10; /* TET10 */
        break;
        case 7:
          ipuf = 3;  /* TRI3  */
        break;
        case 8:
          ipuf = 6;  /* TRI6  */
        break;
        case 9:
          ipuf = 4;  /* QUAD4 */
        break;
        case 10:
          ipuf = 8; /* QUAD8 */
        break;
        case 11:
          ipuf = 2; /* BEAM */
        break;
        case 12:
          ipuf = 3; /* BEAM3 */
        break;
      }
      for (j=0; j<ipuf; j++)
      {
	  //seta( setNr, "n", e_enqire[set[setNr].elem[i]].nod[j] );
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[j];
          set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	//if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[n++]=set[setNr].node[i];
      }
      //set[setNr].anz_n=n+1;
      set[setNr].anz_n=n;
    }
  }
  return(1);
}



int completeSet_Lines( int setNr, int setNrbuf, int *elUsed, int flag )
{
  int i,j;


  if(!flag) /* up  */
  {
    if (set[setNr].anz_p>0)
    {
      /* suche abhaengige linien */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
	//    printf("pnt:%s\n", point[set[setNr].pnt[i]].name);
        for (j=0; j<anzGeo->l; j++)
        {
	  //printf("check line:%s %d  p:%d %d %d\n", line[j].name, elUsed[j], set[setNr].pnt[i], line[j].p1, line[j].p2);
          if( elUsed[j] ) continue;

          if( line[j].name != (char *)NULL )
          if(( set[setNr].pnt[i] == line[j].p1 )||( set[setNr].pnt[i] == line[j].p2 ))
          {
	    //printf("line:%s added\n", line[j].name);
            seta( setNrbuf, "l", j );
          }
        }
      }
    }
  }
  else /* down */
  {
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      j= set[setNr].line[i];
      seta( setNr, "p", line[j].p1 );
      seta( setNr, "p", line[j].p2 );
    }
  }
  return(1);
}



int completeSet( char *setname, char *type )
{
  int i,j;
  int       setNr, k, m, n, p, l, c, s, ipuf;
  int       *nbody=NULL, *nsurf=NULL, *nline=NULL, *npoint=NULL ;

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in completeSet: set:%s does not exist\n", setname);
    return(-1);
  }

  if (toupper(type[0])=='U') /* up  */
  {
    if (set[setNr].anz_n>0)
    {
      /* suche abhaengige elemente */
      m=set[setNr].anz_e;
      for ( i=0; i<anz->e; i++)
      {
        ipuf=0;
        if (e_enqire[e_enqire[i].nr].type == 1) ipuf = 8;       /* HEXA8 */
        else if (e_enqire[e_enqire[i].nr].type == 2) ipuf = 6;  /* PENTA6 */
        else if (e_enqire[e_enqire[i].nr].type == 3) ipuf = 4;  /* TET4 */
        else if (e_enqire[e_enqire[i].nr].type == 4) ipuf = 20; /* HEXA20 */
        else if (e_enqire[e_enqire[i].nr].type == 5) ipuf = 15; /* PENTA15 */
        else if (e_enqire[e_enqire[i].nr].type == 6) ipuf = 10; /* TET10 */
        else if (e_enqire[e_enqire[i].nr].type == 7) ipuf = 3;  /* TRI3  */
        else if (e_enqire[e_enqire[i].nr].type == 8) ipuf = 6;  /* TRI6  */
        else if (e_enqire[e_enqire[i].nr].type == 9) ipuf = 4;  /* QUAD4 */
        else if (e_enqire[e_enqire[i].nr].type == 10) ipuf = 8; /* QUAD8 */
        else if (e_enqire[e_enqire[i].nr].type == 11) ipuf = 2; /* BEAM */
        else if (e_enqire[e_enqire[i].nr].type == 12) ipuf = 3; /* BEAM3 */
        if (ipuf!=0)
        {
          for (j=0; j<ipuf; j++)
          {
            if(ifind(&set[setNr].node, set[setNr].anz_n, e_enqire[e_enqire[i].nr].nod[j])>-1)
            {
	      // seta( setNr, "e", e_enqire[i].nr );
              if ( (set[setNr].elem = (int *)realloc((int *)set[setNr].elem, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
                printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
              set[setNr].elem[set[setNr].anz_e]= e_enqire[i].nr;
              set[setNr].anz_e++;
              break;
            }
          }
        }
      }
      if(set[setNr].anz_e-m)
      {
        qsort( set[setNr].elem, set[setNr].anz_e, sizeof(int), (void *)compareInt );
        /* erase multiple entities */
        n=0;
        for(i=1; i<set[setNr].anz_e; i++)
        {
	  if(set[setNr].elem[n]!=set[setNr].elem[i]) set[setNr].elem[++n]=set[setNr].elem[i];
        }
        set[setNr].anz_e=n+1;
      }
    }
    if (set[setNr].anz_f>0)
    {
      /* suche abhaengige elemente */
      m=set[setNr].anz_e;
      for ( i=0; i<set[setNr].anz_f; i++)
      {
	// seta( setNr, "e", face[set[setNr].face[i]].elem_nr );
        if ( (set[setNr].elem = (int *)realloc((int *)set[setNr].elem, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].elem[set[setNr].anz_e]= face[set[setNr].face[i]].elem_nr;
        set[setNr].anz_e++;
      }
      if(set[setNr].anz_e-m)
      {
        qsort( set[setNr].elem, set[setNr].anz_e, sizeof(int), (void *)compareInt );
        /* erase multiple entities */
        n=0;
        for(i=1; i<set[setNr].anz_e; i++)
        {
	  if(set[setNr].elem[n]!=set[setNr].elem[i]) set[setNr].elem[++n]=set[setNr].elem[i];
        }
        set[setNr].anz_e=n+1;
      }
    }
    if (set[setNr].anz_p>0)
    {
      /* suche abhaengige linien */
      for ( i=0; i<set[setNr].anz_p; i++)
      {
        for (j=0; j<anzGeo->l; j++)
        {
          if( line[j].name != (char *)NULL )
          if(( set[setNr].pnt[i] == line[j].p1 )||( set[setNr].pnt[i] == line[j].p2 ))
          {
            seta( setNr, "l", j );
          }
        }
      }
    }
    if (set[setNr].anz_l>0)
    {
      /* suche abhaengige lcmbs */
      for ( i=0; i<set[setNr].anz_l; i++)
      {
        for (j=0; j<anzGeo->c; j++)
        {
          if( lcmb[j].name != (char *)NULL )
          for (n=0; n<lcmb[j].nl; n++)
          {
            if( set[setNr].line[i] == lcmb[j].l[n] )
            {
              seta( setNr, "c", j );
            }
          }
        }
      }
      /* suche abhaengige surfs */
      for ( i=0; i<set[setNr].anz_l; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          for (n=0; n<surf[j].nl; n++)
          {
            if(( set[setNr].line[i] == surf[j].l[n] )&&( surf[j].typ[n] == 'l' ))
            {
              seta( setNr, "s", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_c>0)
    {
      /* suche abhaengige surfaces */
      for ( i=0; i<set[setNr].anz_c; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          for (n=0; n<surf[j].nl; n++)
          {
            if(( set[setNr].lcmb[i] == surf[j].l[n] )&&( surf[j].typ[n] == 'c' ))
            {
              seta( setNr, "s", j );
            }
          }
        }
      }
    }
    if (set[setNr].anz_s>0)
    {
      /* suche abhaengige bodys */
      for ( i=0; i<set[setNr].anz_s; i++)
      {
        for (j=0; j<anzGeo->b; j++)
        {
          if( body[j].name != (char *)NULL )
          for (n=0; n<body[j].ns; n++)
          {
            if ( set[setNr].surf[i] == body[j].s[n] )
            {
              seta( setNr, "b", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_nurs>0)
    {
      /* suche abhaengige shapes */
      for ( i=0; i<set[setNr].anz_nurs; i++)
      {
        for (j=0; j<anzGeo->sh; j++)
        {
          if(( shape[j].name != (char *)NULL )||( shape[j].type==4))
          {
            if ( set[setNr].nurs[i] == shape[j].p[0] )
            {
              seta( setNr, "sh", j);
            }
          }
        }
      }
    }
    if (set[setNr].anz_sh>0)
    {
      /* suche abhaengige surfaces */
      for ( i=0; i<set[setNr].anz_sh; i++)
      {
        for (j=0; j<anzGeo->s; j++)
        {
          if( surf[j].name != (char *)NULL )
          {
            if ( set[setNr].shp[i] == surf[j].sh )
            {
              seta( setNr, "s", j);
            }
          }
        }
      }
    }
  }

  else if (toupper(type[0])=='D') /* down */
  {
    /* get the faces */
    completeSet( set[setNr].name, "f" );

    /* cyrcle through all nodes and add all geometric entities */
    if(set[setNr].anz_n)
    {
      /* mark the related geometry */
      if( (nbody=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (nsurf=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (nline=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      if( (npoint=(int *)malloc( (anz->nmax+1)*sizeof(int) ) )==NULL) 
      { printf(" ERROR: malloc failure\n"); goto mfail_comp; }
      for (i=0; i<=anz->nmax; i++)
      {
        nbody[i]=nsurf[i]=nline[i]=npoint[i]=-1;
      }

      for (i=0; i<anzGeo->b; i++)
      {
        for (j=0; j<body[i].nn; j++){ nbody[body[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->s; i++)
      {
        for (j=0; j<surf[i].nn; j++){ nsurf[surf[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->l; i++)
      {
        for (j=0; j<line[i].nn; j++){ nline[line[i].nod[j]]=i; }
      }
      for (i=0; i<anzGeo->p; i++)
      {
        for (j=0; j<point[i].nn; j++){ npoint[point[i].nod[j]]=i; }
      }
      for (i=0; i<set[setNr].anz_n; i++)
      {
        /* check if the node is relevant */
        if(nbody[set[setNr].node[i]]>-1) seta( setNr, "b", nbody[set[setNr].node[i]]);
        if(nsurf[set[setNr].node[i]]>-1) seta( setNr, "s", nsurf[set[setNr].node[i]]);
        if(nline[set[setNr].node[i]]>-1) seta( setNr, "l", nline[set[setNr].node[i]]);
        if(npoint[set[setNr].node[i]]>-1) seta( setNr, "p", npoint[set[setNr].node[i]]);
      }

      mfail_comp:;
      free(nbody);
      free(nsurf);
      free(nline);
      free(npoint);
    }

    /* cyrcle through all faces and add all nodes */
    /* nodes have to be completed before because otherwhise all geometry with their nodes would be included */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_f; i++)
    {
      if (face[set[setNr].face[i]].type == 7) n = 3;  /* TRI3  */
      else if (face[set[setNr].face[i]].type == 8) n = 6;  /* TRI6  */
      else if (face[set[setNr].face[i]].type == 9) n = 4;  /* QUAD4 */
      else if (face[set[setNr].face[i]].type == 10) n = 8; /* QUAD8 */
      else if (face[set[setNr].face[i]].type == 11) n = 2; /* BEAM2 */
      else if (face[set[setNr].face[i]].type == 12) n = 3; /* BEAM3 */
      else n=0;
      for (j=0; j<n; j++)
      {
        // seta( setNr, "n", face[set[setNr].face[i]].nod[j] );
        if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].node[set[setNr].anz_n]= face[set[setNr].face[i]].nod[j];
        set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }
    /* circle through all elements and add all nodes and faces */
    /* nodes have to be completed before because otherwhise all geometry with their nodes would be included */
    m=set[setNr].anz_n;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      if (e_enqire[set[setNr].elem[i]].type == 1) n = 8;       /* HEXA8 */
      else if (e_enqire[set[setNr].elem[i]].type == 2) n = 6;  /* PENTA6 */
      else if (e_enqire[set[setNr].elem[i]].type == 3) n = 4;  /* TET4 */
      else if (e_enqire[set[setNr].elem[i]].type == 4) n = 20; /* HEXA20 */
      else if (e_enqire[set[setNr].elem[i]].type == 5) n = 15; /* PENTA15 */
      else if (e_enqire[set[setNr].elem[i]].type == 6) n = 10; /* TET10 */
      else if (e_enqire[set[setNr].elem[i]].type == 7) n = 3;  /* TRI3  */
      else if (e_enqire[set[setNr].elem[i]].type == 8) n = 6;  /* TRI6  */
      else if (e_enqire[set[setNr].elem[i]].type == 9) n = 4;  /* QUAD4 */
      else if (e_enqire[set[setNr].elem[i]].type == 10) n = 8; /* QUAD8 */
      else if (e_enqire[set[setNr].elem[i]].type == 11) n = 2; /* BEAM2 */
      else if (e_enqire[set[setNr].elem[i]].type == 12) n = 3; /* BEAM3 */
      else n=0;
      for (j=0; j<n; j++)
      {
  	  // seta( setNr, "n", e_enqire[set[setNr].elem[i]].nod[j] );
          if ( (set[setNr].node = (int *)realloc((int *)set[setNr].node, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
            printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
          set[setNr].node[set[setNr].anz_n]= e_enqire[set[setNr].elem[i]].nod[j];
          set[setNr].anz_n++;
      }
    }
    if(set[setNr].anz_n-m)
    {
      qsort( set[setNr].node, set[setNr].anz_n, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_n; i++)
      {
	if(set[setNr].node[n]!=set[setNr].node[i]) set[setNr].node[++n]=set[setNr].node[i];
      }
      set[setNr].anz_n=n+1;
    }

    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[setNr].anz_b; i++)
    {
      c= set[setNr].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( setNr, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs and nurbs */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      s= set[setNr].surf[i];
      if(surf[s].sh>-1) seta( setNr, "sh", surf[s].sh );
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( setNr, "l", l );
        }
        else
        {
          seta( setNr, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      c= set[setNr].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( setNr, "l", l );
      }
    }
    /* cyrcle through all lines and add all points, trks */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l= set[setNr].line[i];
      seta( setNr, "p", line[l].p1 );
      seta( setNr, "p", line[l].p2 );
      if (line[l].typ=='a') seta( setNr, "p", line[l].trk );
      if (line[l].typ=='s') seta( setNr, "se", line[l].trk );
    }
    /* cyrcle through all shapes and add all points and nurbs */
    for (i=0; i<set[setNr].anz_sh; i++)
    {
      s= set[setNr].shp[i];
      if(shape[s].type==0) for (j=0; j<3; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==1) for (j=0; j<3; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==2) for (j=0; j<4; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==3) for (j=0; j<2; j++) seta( setNr, "p", shape[s].p[j] );
      else if(shape[s].type==4) seta( setNr, "S", shape[s].p[0] );
    }
    /* cyrcle through all nurbs and add all points */
    for (i=0; i<set[setNr].anz_nurs; i++)
    {
      s= set[setNr].nurs[i];
      for (j=0; j<nurbs[s].u_npnt; j++)
        for (k=0; k<nurbs[s].v_npnt; k++)
          seta( setNr, "p", nurbs[s].ctlpnt[j][k] );
    }
    /* cyrcle through all sets and add all entities */
    for (i=0; i<set[setNr].anz_se; i++)
    {
      s= set[setNr].set[i];
      for (j=0; j<set[s].anz_n; j++)
      {
        p=set[s].node[j];
        seta( setNr, "n", p );
      }
      for (j=0; j<set[s].anz_f; j++)
      {
        p=set[s].face[j];
        seta( setNr, "f", p );
      }
      for (j=0; j<set[s].anz_e; j++)
      {
        p=set[s].elem[j];
        seta( setNr, "e", p );
      }
      for (j=0; j<set[s].anz_p; j++)
      {
        p=set[s].pnt[j];
        seta( setNr, "p", p );
      }
      for (j=0; j<set[s].anz_l; j++)
      {
        p=set[s].line[j];
        seta( setNr, "l", p );
      }
      for (j=0; j<set[s].anz_c; j++)
      {
        p=set[s].lcmb[j];
        seta( setNr, "c", p );
      }
      for (j=0; j<set[s].anz_s; j++)
      {
        p=set[s].surf[j];
        seta( setNr, "s", p );
      }
      for (j=0; j<set[s].anz_b; j++)
      {
        p=set[s].body[j];
        seta( setNr, "b", p );
      }
      for (j=0; j<set[s].anz_sh; j++)
      {
        p=set[s].shp[j];
        seta( setNr, "sh", p );
      }
    }

    /* remove the appended sets */
    while(set[setNr].anz_se>0)
    {
      s= set[setNr].set[0];
      setr( setNr, "r", s);
    }

  }

  else if (toupper(type[0])=='F') /* faces */
  {
    if ( setNr==getSetNr(specialset->mesh)||( set[setNr].type==1)||(setNr==getSetNr(specialset->nomesh))) return(-1);
    
    /* add the faces to the sets which fit fully to the included nodes */
    if(printFlag) printf(" include faces in sets\n");

    /* go over all faces and look if their nodes are included in the set setNr */
    m=set[setNr].anz_f;
    for (i=0; i<anz->f; i++)
    { 
      if (face[i].type == 7) n = 3;  /* TRI3  */
      else if (face[i].type == 8) n = 6;  /* TRI6  */
      else if (face[i].type == 9) n = 4;  /* QUAD4 */
      else if (face[i].type == 10) n = 8; /* QUAD8 */
      else if (face[i].type == 11) n = 2; /* beam2 */
      else if (face[i].type == 12) n = 3; /* beam3 */
      else n=0;
      k=0;
      for (j=0; j<n; j++)
      {
	if(ifind(&set[setNr].node, set[setNr].anz_n, face[i].nod[j])>-1) k++;
      }
      if(k==n)
      {
	// seta( setNr, "f", i );
        if ( (set[setNr].face = (int *)realloc((int *)set[setNr].face, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].face[set[setNr].anz_f]= i;
        set[setNr].anz_f++;
      }
    }
    if(set[setNr].anz_f-m)
    {
      qsort( set[setNr].face, set[setNr].anz_f, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_f; i++)
      {
	if(set[setNr].face[n]!=set[setNr].face[i]) set[setNr].face[++n]=set[setNr].face[i];
      }
      set[setNr].anz_f=n+1;
    }

    /* go over all faces and look if their elements are included in the set setNr */
    m=set[setNr].anz_f;
    for (i=0; i<anz->f; i++)
    { 
      if(ifind(&set[setNr].elem, set[setNr].anz_e, face[i].elem_nr)>-1)
      {
	// seta( setNr, "f", i );
        if ( (set[setNr].face = (int *)realloc((int *)set[setNr].face, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
          printf(" ERROR: malloc failed in set[%d]:%s\n\n", setNr, set[setNr].name);
        set[setNr].face[set[setNr].anz_f]= i;
        set[setNr].anz_f++;
      }
    }
    if(set[setNr].anz_f-m)
    {
      qsort( set[setNr].face, set[setNr].anz_f, sizeof(int), (void *)compareInt );
      /* erase multiple entities */
      n=0;
      for(i=1; i<set[setNr].anz_f; i++)
      {
	if(set[setNr].face[n]!=set[setNr].face[i]) set[setNr].face[++n]=set[setNr].face[i];
      }
      set[setNr].anz_f=n+1;
    }
  }

  else if (toupper(type[0])=='E') /* edges */
  {
    /* cyrcle through all bodys and add all surfs */
    for (i=0; i<set[setNr].anz_b; i++)
    {
      c= set[setNr].body[i];
      for (j=0; j<body[c].ns; j++)
      {
        l=body[c].s[j];
        seta( setNr, "s", l );
      }
    }
    /* cyrcle through all surfs and add all lines, lcmbs  */
    for (i=0; i<set[setNr].anz_s; i++)
    {
      s= set[setNr].surf[i];
      for (j=0; j<surf[s].nl; j++)
      {
        l=surf[s].l[j];
        if (surf[s].typ[j]=='l')
        {
          seta( setNr, "l", l );
        }
        else
        {
          seta( setNr, "c", l );
        }
      }
    }
    /* cyrcle through all lcmbs and add all lines */
    for (i=0; i<set[setNr].anz_c; i++)
    {
      c= set[setNr].lcmb[i];
      for (j=0; j<lcmb[c].nl; j++)
      {
        l=lcmb[c].l[j];
        seta( setNr, "l", l );
      }
    }
    /* cyrcle through all lines and add all corner-points */
    for (i=0; i<set[setNr].anz_l; i++)
    {
      l= set[setNr].line[i];
      seta( setNr, "p", line[l].p1 );
      seta( setNr, "p", line[l].p2 );
    }
  }
  else printf(" Command not recognized, use up down or edges\n");
  return(setNr);
}



/* send the interiour of the surfaces in stl format (not used ) */

int sendTriangles(  char *setname )
{
  int i,j,k,n,s, setNr, tris=0;
  FILE *handle;

  operateAlias( setname, "se" );
  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in completeSet: set:%s does not exist\n", setname);
    return(-1);
  }

  sprintf(buffer, "%s.stl", setname);
  handle = fopen (buffer, "w");
  if (handle==NULL)
  {
    printf (" ERROR: The input file \"%s\" could not be opened.\n\n", datin);
    return(-1);
  }
  else  printf ("\n%s opened\n",buffer);

  /* write the interiour of the surface */
  fprintf( handle, "solid\n");
  for (i=0; i<set[setNr].anz_s; i++)
  {
    s= set[setNr].surf[i];

    n=0;
    while((surf[s].npgn-n))
    {
      n++;
      j=surf[s].pgn[n++];
      fprintf( handle, "  facet normal %e %e %e\n",surf[s].pgn[n],surf[s].pgn[n+1],surf[s].pgn[n+2]); n+=3;
      fprintf( handle, "    outer loop\n");
      for(k=0; k<j; k++)
      {
        fprintf( handle, "      vertex %e %e %e\n",
          surf[s].pgn[n]*scale->w+scale->x,
          surf[s].pgn[n+1]*scale->w+scale->y,
          surf[s].pgn[n+2]*scale->w+scale->z); n+=3;
      }
      tris++;
      fprintf( handle, "    endloop\n");
      fprintf( handle, "  endfacet\n");
    }
  }
  fprintf( handle, "endsolid\n");
  printf("wrote %d triangles\n", tris);
  fclose(handle);
  return(1);
}



void pre_write( char *record )
{
  char  setname[MAX_LINE_LENGTH], format[MAX_LINE_LENGTH], type[MAX_LINE_LENGTH], **dat;
  int          length, anz_l, setNr, i,j,k,l=0,s,v,n,b;
  char typ, buffer[MAX_LINE_LENGTH];
  static char  **val=NULL;
  double        f[3]={0.,0.,0.};
  int bset;
  int bouNr;
  Dunsbou *dunsbou=NULL;
  int *linebuf=NULL, *lcmbbuf=NULL, *surfbuf=NULL;
  double v1[3], v2[3], v3[3];
  int tr6[4][3]={{0,3,5},{4,5,3},{3,1,4},{4,2,5}};
  int qu4[2][3]={{0,1,2},{2,3,0}};
  int qu8[8][3]={{0,4,8},{8,4,1},{1,5,8},{8,5,2},{2,6,8},{8,6,3},{3,7,8},{8,7,0}};

  FILE *handle=NULL;

  if(val){ for(i=0; i<5; i++) free(val[i]);  free(val); }
  if(( val=(char **)malloc( 5*sizeof(char *)) )==NULL)
  { printf(" ERROR: malloc failure\n\n" ); return; }
  for(i=0; i<5; i++)
  {
    if(( val[i]=(char *)malloc( MAX_LINE_LENGTH*sizeof(char)) )==NULL)
    { printf(" ERROR: malloc failure\n\n" ); return; }
    val[i][0]=0;
  }

  /* clean the buffers */
  for(i=0; i<MAX_LINE_LENGTH; i++) buffer[i]=setname[i]=format[i]=type[i]='\0';
  for(i=0; i<MAX_LINE_LENGTH; i++) for(j=0; j<5; j++) val[j][i]='\0';

  length = sscanf(record, "%s%s%s%s%s%s%s%s", setname, format, type, val[0], val[1], val[2], val[3], val[4]);
  if (length<2) { printf(" ERROR: format missing!\n"); return; }

  /* replace 'lc' by 'ds' */
  if(compare( type, "lc", 2)== 2) { type[0]='d'; type[1]='s'; }
  for(i=0; i<4; i++) if(compare( val[i], "lc", 2)== 2) { val[i][0]='d'; val[i][1]='s'; }

  operateAlias( setname, "se" );
  printf (" please wait for 'ready'\n");

  if (compare( format, "bp", 2)== 2)
  {
    if(getSetNr(specialset->zap)>-1)
    {
      errMsg("ERROR: special set:%s is not empty. Please delete the set before\n", specialset->zap);
      return;
    }
    if (length>2)
    {
      if (compare( type, "c", 1)== 1)
      {
      completeSet( setname, "do" );
      }
    }
    descalPoints( anzGeo->p, point, scale);
    writebp( setname, anz, anzGeo, point, set );
    scalPoints( anzGeo->p, point, scale);
  }
  else if (compare( format, "fbd", 3)== 3)
  {
    if(getSetNr(specialset->zap)>-1)
    {
      errMsg("ERROR: special set:%s is not empty. Please delete the set before\n", specialset->zap);
      return;
    }
    if (length>2)
    {
      if (compare( type, "c", 1)== 1)
      {
        completeSet( setname, "do" );
      }
      if (compare( type, "e", 1)== 1)
      {
        sprintf(buffer,"%s_edges.fbd", setname);
        handle = fopen (buffer, "w+b");
        if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", buffer ); return; }
        descalNodes ( anz->n, node, scale);
        for (i=0; i<anz->g; i++ )
        {
          fprintf(handle, " pnt a%d %f %f %f\n", i, node[edge[i].p1].nx, node[edge[i].p1].ny, node[edge[i].p1].nz );
          fprintf(handle, " pnt b%d %f %f %f\n", i, node[edge[i].p2].nx, node[edge[i].p2].ny, node[edge[i].p2].nz );
          fprintf(handle, " line l%d a%d b%d 1\n",i,i,i);
        }
        fprintf(handle, " merg p all 0.00001\n");
        scalNodes ( anz->n, node, scale );
        fclose(handle);
        printf (" ready\n");
        return;
      }
    }
    descalPoints( anzGeo->p, point, scale);
    writefbd( setname, anz, anzGeo, point, line, lcmb, surf, body, nurbs, set, shape, value );
    scalPoints( anzGeo->p, point, scale);
  }
  else if (compare( format, "ng", 2)== 2)
  {
    if(compareStrings( setname, "all" )<=0)
    {
	printf("ERROR: netgen-format was requested for a subset:%s. Up to now only the faces of the full mesh can be written (set all)\n",setname);
      return;
    }
    if(anz->f<1)
    {
      printf("ERROR: no faces exist\n");
      return;
    }

    descalNodes ( anz->n, node, scale);

    handle = fopen ("mesh.ng", "w+b");
    if (handle==NULL) { printf ("\nThe output file \"mesh.ng\" could not be opened.\n\n"); return; }
    printf (" write surface-mesh data for the netgen-tetmesher ng_vol \n");

    fprintf (handle, "%d\n", anz->n);
    for (i=0; i<anz->n; i++)
    {
      fprintf (handle, "%.12e %.12e %.12e\n", node[node[i].nr].nx, node[node[i].nr].ny, node[node[i].nr].nz);
    }
    i=0;
    for (j=0; j<anz->f; j++)
    {
      if ((face[j].type == 7)|| (face[j].type == 8)) i++;
      else if ((face[j].type == 9)|| (face[j].type == 10)) i+=2;
    }
    fprintf (handle, "%d\n", i);
/*
    fprintf (handle, "%d\n", anz->e);
    for (i=0; i<anz->e; i++)
    {
      j=e_enqire[i].nr;
      if (e_enqire[j].type == 7)
      {
        fprintf (handle, "%d %d %d\n",
         node[e_enqire[j].nod[0]].indx+1, node[e_enqire[j].nod[1]].indx+1, node[e_enqire[j].nod[2]].indx+1);
      }
      else
      {
        printf (" WARNING: elem(%d) not a known type (%d)\n", e_enqire[j].nr, e_enqire[j].type);
      }
    }
*/
    for (j=0; j<anz->f; j++)
    {
      if ((face[j].type == 7)|| (face[j].type == 8))
      {
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[0]].indx+1, node[face[j].nod[1]].indx+1, node[face[j].nod[2]].indx+1);
      }
      else if ((face[j].type == 9)|| (face[j].type == 10))
      {
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[0]].indx+1, node[face[j].nod[1]].indx+1, node[face[j].nod[2]].indx+1);
        fprintf (handle, "%d %d %d\n",
         node[face[j].nod[2]].indx+1, node[face[j].nod[3]].indx+1, node[face[j].nod[0]].indx+1);
      }
      else
      {
        printf (" WARNING: face(%d) not a known type (%d)\n", j, face[j].type);
      }
    }
    fclose(handle);

    scalNodes ( anz->n, node, scale);
  }
  else if (compare( format, "frd", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "nor", 3)== 3)
        sendSurfNormalen( setname, format, anz, node, e_enqire, lcase, scale );
      else if (compare( type, "sur", 3)== 3)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[1],val[0]);
        strcpy(val[0],type);
	//val[1]=val[0];
        //val[0]=type;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else if ((compare( type, "ds", 2)== 2)||(compare( type, "bin", 2)== 2))
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, val);
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "darwin", 3)== 3)
  {
    anz_l=anz->l;
    anz->l=anz->olc;
    strcpy(val[1], val[0]);
    strcpy(val[0], type);
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
    anz->l=anz_l;
  }
  /* list of entities */
  else if (compare( format, "lst", 3)== 3)
  {
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
  }
  else if (compare( format, "seq", 3)== 3)
  {
    senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
  }
  /* gagemap */
  else if (compare( format, "gmp", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    return;
  }
  /* patran neutral file */
  else if (compare( format, "pat", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
      {
        /* if setname == "all" -> send all sets */
        if(compareStrings( setname, "all" )>0)
        {
          for(i=0; i<anz->sets; i++)
	  {
            if((set[i].name!=NULL)&&(set[i].name[0]!='+')&&(set[i].name[0]!='-')&&(compareStrings(set[i].name, "all" )<=0)&&((set[i].anz_n>0)||(set[i].anz_e>0))) sendNames( set[i].name, format, anz, node, e_enqire );
	  }
	} 
        sendNames( setname, format, anz, node, e_enqire );
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    return;
  }
  else if (compare( format, "abq", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "quadlin", 3)== 3)
        sendQuadLin( setname, format, anz, node, e_enqire );
      else if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "film", 3)== 3)
        sendFilm( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3], val[4] );
      else if (compare( type, "radiate", 3)== 3)
        sendRadiate( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3], val[4] );
      else if (compare( type, "mflow", 5)== 5)
        sendDflux( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 1 );
      else if (compare( type, "dflux", 5)== 5)
        sendDflux( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "force", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spcf", 4)== 4)
        sendSPCF( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "cflux", 3)== 3)
        sendCflux( setname, format, anz, node, e_enqire, "t", val[0] );
      else if (compare( type, "slidef", 6)== 6)
        sendSlidersf( setname, format, val[0]  );
      else if (compare( type, "slide", 3)== 3)
        sendSliders( setname, format, anz, node, e_enqire, val[0]  );
      else if (compare( type, "names", 3)== 3)
      {
        /* if setname == "all" -> send all sets */
        if(compareStrings( setname, "all" )>0)
        {
          for(i=0; i<anz->sets; i++)
	  {
            if((set[i].name!=NULL)&&(set[i].name[0]!='+')&&(set[i].name[0]!='-')&&(compareStrings(set[i].name, "all" )<=0)&&((set[i].anz_n>0)||(set[i].anz_e>0))) sendNames( set[i].name, format, anz, node, e_enqire );
	  }
	} 
        sendNames( setname, format, anz, node, e_enqire );
      }
      else if (compare( type, "sur", 3)== 3)
        sendSurfaces( setname, format, anz, node, e_enqire, val[0] );
      else if (compare( type, "mpc", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendMpc( setname, format, val[0], f ); }
      else if (compare( type, "ds", 2)== 2)
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "tmf", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "crp", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[3], val[2]); /* write-frequency */
        strcpy(val[2], val[1]); /* reference-speed */
        strcpy(val[1], val[0]); /* time-factor */
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "sta", 2)== 2)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        strcpy(val[2], val[0]); /* reference-speed */
        strcpy(val[0], type);
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== strlen(type))
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  /* changes from Paul CARRICO */
  else if (compare( format, "aster", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  /* changes from Paul CARRICO */
  else if (compare( format, "sam", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "adh", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "ans", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "slide", 3)== 3)
        sendSliders( setname, format, anz, node, e_enqire, val[0]  );
      else if (compare( type, "names", 3)== 3)
        sendNames( setname, format, anz, node, e_enqire );
      else if (compare( type, "ds", 2)== 2)
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "nas", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else if (compare( type, "mpc", 3)== 3)
      { 
        length = sscanf(record, "%*s%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendMpc( setname, format, val[0], f ); 
      }
      else if (compare( type, "ds", 2)== 2)
      {
        strcpy(val[1], val[0]);
        strcpy(val[0], type);
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, val);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "tcg", 3)== 3)
  {
    if (length>2)
    {
      if (compare( type, "pres", 3)== 3)
        sendPressure( setname, format, anz, node, e_enqire, val[0], val[1], val[2], 0 );
      else if (compare( type, "for", 3)== 3)
      { length = sscanf(record, "%*s%*s%*s%lf%lf%lf", &f[0],&f[1],&f[2]);
        sendForce( setname, format, anz, node, e_enqire, f ); }
      else if (compare( type, "spc", 3)== 3)
        sendSPC( setname, format, anz, node, e_enqire, val[0], val[1], val[2], val[3] );
      else if (compare( type, "c", 1)== 1)
      {
        anz_l=anz->l;
        anz->l=anz->olc;
        senddata( setname, format, anz, node, e_enqire, lcase, scale, 1, 0);
        anz->l=anz_l;
      }
      else
      {
        errMsg(" ERROR:  %s in format %s not yet supported\n", type, format );
        return;
      }
    }
    else
    {
      anz_l=anz->l;
      anz->l=anz->olc;
      senddata( setname, format, anz, node, e_enqire, lcase, scale, 0, 0);
      anz->l=anz_l;
    }
  }
  else if (compare( format, "dolfyn", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);
    descalNodes ( anz->n, node, scale);
    write2dolfyn( setname, length-2, &dat[2], anz, node, face, e_enqire, set );
    scalNodes ( anz->n, node, scale);

    /* free dat */
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "duns", 3)== 3)
  {
    if (compare( type, "names", 3)== 3) sendNames( setname, format, anz, node, e_enqire );
    else
    {
      length= strsplt( record, ' ', &dat);

      /* initialize the bctype with "0" */
      for( b=0; b<anz->b; b++)
      {
        if(nBlock[b].dim==2)
        {
          s=nBlock[b].geo;
          for(j=0; j<surf[s].nl; j++) nBlock[b].bctype[j][0]=0;
        }
        if(nBlock[b].dim==3)
        {
          s=nBlock[b].geo;
          for(j=0; j<body[s].ns; j++) nBlock[b].bctype[j][0]=0;
        }
      }

      /* add the grid-blocks to the type of boundary */
      bouNr=-1;
      n=2; while(n<length)
      {
        /* search the corresponding blocks */
        bset=getSetNr(dat[n+1]);
        if(bset<0)
        {
          printf("boundary set:%s does not exist\n", dat[n+1]);
          return;
        }
        bouNr++;
        if((dunsbou=(Dunsbou *)realloc((Dunsbou *)dunsbou, (bouNr+1)*sizeof(Dunsbou)) )==NULL)
        { printf(" ERROR: realloc failure, dunsbou:%d could not be allocated\n\n", bouNr ); return; }
        strcpy(dunsbou[bouNr].name,set[bset].name);
        dunsbou[bouNr].surf=NULL;
        dunsbou[bouNr].surfs=0;

        /* add the lcmb to the set */
        for(i=0; i<set[bset].anz_l; i++)
        {
          for(j=0; j<anzGeo->c; j++)
          {
            for(l=0; l<lcmb[j].nl; l++)
            {
              if(lcmb[j].l[l]==set[bset].line[i]) seta(bset, "c", j);
            }
          }
        }
  
        if(( linebuf=(int *)realloc((int *)linebuf, (anzGeo->l+2)*sizeof(int)) )==NULL)
        { printf(" ERROR: realloc failure\n\n" ); return; }
        if(( lcmbbuf=(int *)realloc((int *)lcmbbuf, (anzGeo->c+2)*sizeof(int)) )==NULL)
        { printf(" ERROR: realloc failure\n\n" ); return; }
        if(( surfbuf=(int *)realloc((int *)surfbuf, (anzGeo->s+2)*sizeof(int)) )==NULL)
        { printf(" ERROR: realloc failure\n\n" ); return; }
        for(i=0; i<set[bset].anz_l; i++)        linebuf[set[bset].line[i]]=-1;
        for(i=0; i<set[bset].anz_c; i++)        lcmbbuf[set[bset].lcmb[i]]=-1;
        for(i=0; i<set[bset].anz_s; i++)        surfbuf[set[bset].surf[i]]=-1;

        /* look if a member of the set is used by a block */
        for( b=0; b<anz->b; b++)
        {
          if(nBlock[b].dim==2)
          {
            s=nBlock[b].geo;
            for(i=0; i<set[bset].anz_l; i++)
            {
              for(j=0; j<surf[s].nl; j++)
              {
                if(nBlock[b].map[j][0]==-1)
                {
                  l=surf[s].l[nBlock[b].bcface[j]];
                  typ=surf[s].typ[nBlock[b].bcface[j]];
                }
                else typ=-1;
                if(typ=='l')
                {
                  if(l==set[bset].line[i])
                  {
                    linebuf[l]=nBlock[b].neighbor[j];
                    break;
                  }
                }
              }
            }
            for(i=0; i<set[bset].anz_c; i++)
            {
              for(j=0; j<surf[s].nl; j++)
              {
                if(nBlock[b].map[j][0]==-1)
                {
                  typ=surf[s].typ[nBlock[b].bcface[j]];
                  l=surf[s].l[nBlock[b].bcface[j]];
                }
                else typ=-1;
                if(typ=='c')
                {
                  if(l==set[bset].lcmb[i])
                  {
                    lcmbbuf[l]=nBlock[b].neighbor[j];
                    break;
                  }
                }
              }
            }
          }
          if(nBlock[b].dim==3)
	  {
            v=nBlock[b].geo;
            for(i=0; i<set[bset].anz_s; i++)
            {
              for(j=0; j<body[v].ns; j++)
              {
                if(nBlock[b].map[j][0]==-1) s=body[v].s[nBlock[b].bcface[j]];
                else s=-1;
                if(s==set[bset].surf[i])
                {
            printf("body:%s bsurf:%s surf:%s\n", body[v].name, surf[body[v].s[nBlock[b].bcface[j]]].name, surf[set[bset].surf[i]].name);
                  surfbuf[s]=nBlock[b].neighbor[j];
                  break;
                }
              }
            }
	  }
        }

        /* store the identified entities in dunsbou */
        for(i=0; i<set[bset].anz_l; i++)
        {
          if(linebuf[set[bset].line[i]]>0)
	  {
            if((dunsbou[bouNr].surf=(int *)realloc((int *)dunsbou[bouNr].surf, (dunsbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
            { printf(" ERROR: realloc failure, dunsbou:%d surface could not be allocated\n\n", bouNr ); return; }
            printf("set:%s bou:%s bouNr:%d store line[%d] %s in %s nr:%d\n",set[bset].name, dat[n],bouNr,dunsbou[bouNr].surfs, line[set[bset].line[i]].name, dat[n],linebuf[set[bset].line[i]]);
            dunsbou[bouNr].surf[dunsbou[bouNr].surfs] = linebuf[set[bset].line[i]];
            strcpy(dunsbou[bouNr].bctype, dat[n]);
            dunsbou[bouNr].surfs++;
	  }
	}
        for(i=0; i<set[bset].anz_c; i++)
        {
          if(lcmbbuf[set[bset].lcmb[i]]>0)
	  {
            if((dunsbou[bouNr].surf=(int *)realloc((int *)dunsbou[bouNr].surf, (dunsbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
            { printf(" ERROR: realloc failure, dunsbou:%d surface could not be allocated\n\n", bouNr ); return; }
            printf("set:%s bou:%s bouNr:%d store lcmb[%d] %s in %s nr:%d\n",set[bset].name, dat[n],bouNr,dunsbou[bouNr].surfs, lcmb[set[bset].lcmb[i]].name, dat[n],lcmbbuf[set[bset].lcmb[i]]);
            dunsbou[bouNr].surf[dunsbou[bouNr].surfs] = lcmbbuf[set[bset].lcmb[i]];
            strcpy(dunsbou[bouNr].bctype, dat[n]);
            dunsbou[bouNr].surfs++;
	  }
	}
        for(i=0; i<set[bset].anz_s; i++)
        {
          if(surfbuf[set[bset].surf[i]]>0)
	  {
            if((dunsbou[bouNr].surf=(int *)realloc((int *)dunsbou[bouNr].surf, (dunsbou[bouNr].surfs+2)*sizeof(int)) )==NULL)
            { printf(" ERROR: realloc failure, dunsbou:%d surface could not be allocated\n\n", bouNr ); return; }
            printf("set:%s bou:%s bouNr:%d store surf[%d] %s in %s nr:%d\n",set[bset].name, dat[n],bouNr,dunsbou[bouNr].surfs, surf[set[bset].surf[i]].name, dat[n],surfbuf[set[bset].surf[i]]);
            dunsbou[bouNr].surf[dunsbou[bouNr].surfs] = surfbuf[set[bset].surf[i]];
            strcpy(dunsbou[bouNr].bctype, dat[n]);
            dunsbou[bouNr].surfs++;
	  }
	}

        n+=2;
      }
      bouNr++;

      descalNodes ( anz->n, node, scale);
      write2duns( setname, anz, node, e_enqire, lcase, nBlock, bouNr, dunsbou );
      scalNodes ( anz->n, node, scale);

      for(i=0; i<bouNr; i++) free(dunsbou[i].surf); free(dunsbou);
      free(linebuf);
      free(lcmbbuf);
      free(surfbuf);

      /* free dat */
      for(i=0; i<length; i++) free(dat[i]); free(dat);
    }
  }
  else if (compare( format, "foam", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);
    descalNodes ( anz->n, node, scale);
    write2foam( setname, length-2, &dat[2], anz, node, face, e_enqire, set, lcase );
    scalNodes ( anz->n, node, scale);

    /* free dat */
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "isaac", 3)== 3)
  {
    length= strsplt( record, ' ', &dat);

    for( b=0; b<anz->b; b++)
    {
      if(nBlock[b].dim==2)
      {
        s=nBlock[b].geo;
        for(j=0; j<surf[s].nl; j++) strcpy(nBlock[b].bctype[j], "not defined");
      }
      if(nBlock[b].dim==3)
      {
        s=nBlock[b].geo;
        for(j=0; j<body[s].ns; j++) strcpy(nBlock[b].bctype[j], "not defined");
      }
    }
    /* add the type of boundary to the grid-blocks */
    n=2; while(n<length)
    {
      /* search the corresponding blocks */
      bset=getSetNr(dat[n+1]);
      if(bset<0)
      {
        printf("boundary set:%s does not exist\n", dat[n]);
        return;
      }

      if(nBlock[b].dim==2)
      {
       /* add the lcmb to the set */
       for(i=0; i<set[bset].anz_l; i++)
       {
        for(j=0; j<anzGeo->c; j++)
        {
          for(l=0; l<lcmb[j].nl; l++)
	  {
	    printf("add lcmb:%s to set %s\n", lcmb[j].name, set[bset].name);
            if(lcmb[j].l[l]==set[bset].line[i]) seta(bset, "c", j);
	  }
	}
       }
      }

      /* look if a member of the set is used by a block */
      for( b=0; b<anz->b; b++)
      {
        if(nBlock[b].dim==2)
        {
          s=nBlock[b].geo;
          for(i=0; i<set[bset].anz_l; i++)
          {
            for(j=0; j<surf[s].nl; j++)
            {
              l=surf[s].l[nBlock[b].bcface[j]];
              typ=surf[s].typ[nBlock[b].bcface[j]];
              if(typ=='l')
              {
                if(l==set[bset].line[i])
                {
                  printf(" store line %s in set:%s\n",line[l].name ,dat[n]);
                  strcpy(nBlock[b].bctype[j], dat[n]);
                }
              }
            }
          }
          for(i=0; i<set[bset].anz_c; i++)
          {
            for(j=0; j<surf[s].nl; j++)
            {
              l=surf[s].l[nBlock[b].bcface[j]];
              typ=surf[s].typ[nBlock[b].bcface[j]];
              if(typ=='c')
              {
                if(l==set[bset].lcmb[i])
                {
                  printf(" store lcmb %s in set:%s \n",lcmb[l].name, dat[n]);
                  strcpy(nBlock[b].bctype[j], dat[n]);
                }
              }
            }
          }
        }
        if(nBlock[b].dim==3)
        {
          s=nBlock[b].geo;
          for(i=0; i<set[bset].anz_s; i++)
          {
            for(j=0; j<body[s].ns; j++)
            {
              l=body[s].s[nBlock[b].bcface[j]];
              if(l==set[bset].surf[i])
              {
                printf(" store surf %s in set:%s\n", surf[l].name, dat[n]);
                strcpy(nBlock[b].bctype[j], dat[n]);
              }
            }
          }
        }
      }
      n+=2;
    }

    for( b=0; b<anz->b; b++)
    {
      if(nBlock[b].dim==2)
      {
        s=nBlock[b].geo;
        for(j=0; j<surf[s].nl; j++)
        {
          if((surf[s].typ[nBlock[b].bcface[j]]=='l')&&(nBlock[b].bctype[j][0]==0))
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",line[surf[s].l[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], b);
          } 
          if((surf[s].typ[nBlock[b].bcface[j]]=='c')&&(nBlock[b].bctype[j][0]==0))
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",lcmb[surf[s].l[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], b);
          } 
	}
      }
      if(nBlock[b].dim==3)
      {
        s=nBlock[b].geo;
        for(j=0; j<body[s].ns; j++)
        {
          if(nBlock[b].bctype[j][0]==0)
          {
            printf(" WARNING: free surface:%s are not defined as boundary:%s in block: %d\n",surf[body[s].s[nBlock[b].bcface[j]]].name,nBlock[b].bctype[j], b);
          } 
	}
      }
    }

    descalNodes ( anz->n, node, scale);   
    write2isaac( setname, anz, node, e_enqire, lcase, nBlock );
    scalNodes ( anz->n, node, scale);

    /* free dat */
    for(i=0; i<length; i++) free(dat[i]); free(dat);
  }
  else if (compare( format, "stl", 3)== 3)
  {
    if(compareStrings( setname, "all" )<=0)
    {
      printf("ERROR: netgen-format was requested for a subset:%s. Up to now only the faces of the full mesh can be written (set all)\n",setname);
      return;
    }
    if(anz->f<1)
    {
      printf("ERROR: no faces exist\n");
      return;
    }

    descalNodes ( anz->n, node, scale);

    handle = fopen ("mesh.stl", "w+b");
    if (handle==NULL) { printf ("\nThe output file \"mesh.stl\" could not be opened.\n\n"); return; }
    printf (" write surface-mesh data in stl format \n");

    fprintf( handle, "solid\n");
    for (i=0; i<anz->f; i++)
    {
      if (face[i].type == 7)
      {
        v_result( &node[face[i].nod[0]].nx, &node[face[i].nod[1]].nx, v1);
        v_result( &node[face[i].nod[1]].nx, &node[face[i].nod[2]].nx, v2);
        v_result( &node[face[i].nod[2]].nx, &node[face[i].nod[0]].nx, v3);
        if(v_betrag(v1)==0) continue;
        if(v_betrag(v2)==0) continue;
        if(v_betrag(v3)==0) continue;
        v_prod( v1, v2, v3 );
        if(v_norm( v3, v1 )==0) continue;
        fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
        fprintf( handle, "    outer loop\n");
        for(k=0; k<3; k++)
        {
          fprintf( handle, "      vertex %e %e %e\n",
          node[face[i].nod[k]].nx,
          node[face[i].nod[k]].ny,
          node[face[i].nod[k]].nz);
        }
        fprintf( handle, "    endloop\n");
        fprintf( handle, "  endfacet\n");
      }
      else if (face[i].type == 8)
      {
        for(n=0; n<4; n++)
        {
          v_result( &node[face[i].nod[tr6[n][0]]].nx, &node[face[i].nod[tr6[n][1]]].nx, v1);
          v_result( &node[face[i].nod[tr6[n][1]]].nx, &node[face[i].nod[tr6[n][2]]].nx, v2);
          v_result( &node[face[i].nod[tr6[n][2]]].nx, &node[face[i].nod[tr6[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[tr6[n][k]]].nx,
            node[face[i].nod[tr6[n][k]]].ny,
            node[face[i].nod[tr6[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
      else if (face[i].type == 9)
      {
        for(n=0; n<2; n++)
        {
          v_result( &node[face[i].nod[qu4[n][0]]].nx, &node[face[i].nod[qu4[n][1]]].nx, v1);
          v_result( &node[face[i].nod[qu4[n][1]]].nx, &node[face[i].nod[qu4[n][2]]].nx, v2);
          v_result( &node[face[i].nod[qu4[n][2]]].nx, &node[face[i].nod[qu4[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[qu4[n][k]]].nx,
            node[face[i].nod[qu4[n][k]]].ny,
            node[face[i].nod[qu4[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
      else if (face[i].type ==10)
      {
        for(n=0; n<8; n++)
        {
          v_result( &node[face[i].nod[qu8[n][0]]].nx, &node[face[i].nod[qu8[n][1]]].nx, v1);
          v_result( &node[face[i].nod[qu8[n][1]]].nx, &node[face[i].nod[qu8[n][2]]].nx, v2);
          v_result( &node[face[i].nod[qu8[n][2]]].nx, &node[face[i].nod[qu8[n][0]]].nx, v3);
          if(v_betrag(v1)==0) continue;
          if(v_betrag(v2)==0) continue;
          if(v_betrag(v3)==0) continue;
          v_prod( v1, v2, v3 );
          if(v_norm( v3, v1 )==0) continue;
          fprintf( handle, "  facet normal %e %e %e\n",v1[0],v1[1],v1[2]);
          fprintf( handle, "    outer loop\n");
          for(k=0; k<3; k++)
          {
            fprintf( handle, "      vertex %e %e %e\n",
            node[face[i].nod[qu8[n][k]]].nx,
            node[face[i].nod[qu8[n][k]]].ny,
            node[face[i].nod[qu8[n][k]]].nz);
          }
          fprintf( handle, "    endloop\n");
          fprintf( handle, "  endfacet\n");
        }
      }
    }
    fprintf( handle, "endsolid\n");
    fclose(handle);

    /* write netgen Edges */

    /* write the elements if they are be2 */
    j=0;
    for (i=0; i<anz->e; i++) if (e_enqire[e_enqire[i].nr].type == 11) j++;
    if(j)
    {
      handle = fopen ("mesh.ned", "w+b");
      if (handle==NULL) { printf ("\nThe output file \"mesh.ned\" could not be opened.\n\n"); return; }
      printf (" write edge data in ng format \n");
      fprintf( handle, "%d\n",j);
      printf("write %d edges\n", j);

      for (i=0; i<anz->e; i++)
      {
        if (e_enqire[e_enqire[i].nr].type == 11)
        {
          fprintf( handle, "2 %e %e %e %e %e %e\n",
          node[e_enqire[e_enqire[i].nr].nod[0]].nx,
          node[e_enqire[e_enqire[i].nr].nod[0]].ny,
          node[e_enqire[e_enqire[i].nr].nod[0]].nz,
          node[e_enqire[e_enqire[i].nr].nod[1]].nx,
          node[e_enqire[e_enqire[i].nr].nod[1]].ny,
          node[e_enqire[e_enqire[i].nr].nod[1]].nz);
        }
      }
      fclose(handle);
    }

    scalNodes ( anz->n, node, scale);
  }
  else
  {
    /* assume 'format' is  a second set for the mpc's */
    setNr=getSetNr(setname);
    if (setNr<0)
    {
      printf (" ERROR: set:%s does not exist\n", setname);
      return;
    }
    i=getSetNr(format);
    if (i>-1)
    {
       length = sscanf(record, "%*s%*s%s%s%s%s", format, type, val[0], buffer);
       if (compare(type, "cycmpcf", 7)==7) cycmpc(setNr, i, "abqf", val[0], buffer);
       else if (compare(type, "cycmpc", 6)==6) cycmpc(setNr, i, format, val[0], buffer);
       else if (compare(type, "gap", 3)==3) gap(record);
       else if (compare(type, "areampc", 7)==7) { areampc(setNr, i, format, type, val[0], buffer, 0, node, 1); }
       else
       { 
         errMsg(" ERROR: mpc type %s not yet supported, available are cycmpc or areampc\n", type );
         return;
       }
       /* the additional nodes for the HE20 (then HE26) elements need a new pos.  */
       adjustDrawNodes(0);
    }
    else errMsg(" ERROR: format:%s not yet supported, or set:%s does not exist\n", format, format );
  }
  printf (" ready\n");
}



/*------------------------------------------------------------------*/
/* Netzdaten im frd, abaqus oder nastran format rausschreiben       */
/*------------------------------------------------------------------*/

/* syntax: 1,2-8,12, 2-8+2 */
void lcparser(char *string, int *lc)
{
  int i, i1=0, j, val,pos,inc;
  char abuf[134];

  j=0;
  pos = strlen(string);

  i=0;
  while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
  abuf[i-i1]=0;
  printf("val:%s\n", abuf);
  val=atoi(abuf);
  j=val-1;
  lc[j]=1;
  for(i=i; i<=pos; i++)
  {
    if(string[i]==',')
    {
      i++;
      i1=i;
      while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
      abuf[i-i1]=0;
      i--;
      val=atoi(abuf);
      printf("val:%d\n",val );
      j=val-1;
      lc[j]=1;
    }
    else if(string[i]=='-')
    {
      i++;
      i1=i;
      while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; i++;}
      abuf[i-i1]=0;
      val=atoi(abuf);
      if(string[i]=='+')
      {
        i++;
        i1=i;
        while((string[i]>47)&&(string[i]<58)) { abuf[i-i1]=string[i]; printf("i:%d %d abuf:%c\n", i,i1,abuf[i-i1]); i++;}
        abuf[i-i1]=0;
        inc=atoi(abuf);
      }
      else inc=1;
      i--;
      // printf("inc:%d\n", inc);
      printf("val:%d\n", val);
      for (;j<val;j+=inc) { lc[j]=1; }
    }
    else if(string[i]==0) printf("parsed\n"); 
    else
    {
      printf("ERROR: sign:%c not known\n", string[i]);
    }
  }
}



void write2lst( char *datout, Summen *anz, Nodes *node, Elements *elem)
{
  FILE *handle1;
  int  i,n=0;

  handle1 = fopen (datout, "w+b");
  if (handle1==NULL) 
  {
    printf ("\nThe output file \"%s\" could not be opened.\n\n",datout);
    return;
  }
  else  printf (" file %s opened\n",datout);

  printf ("\n write list(lst) file  \n");

  fprintf (handle1, "# nodes\n");
  for (i=0; i<anz->n; i++)
  {
    n++;
    fprintf (handle1, " %d", node[i].nr);
    if(n>6) { n=0; fprintf (handle1, "\n"); }
  }
  fprintf (handle1, "\n# elements\n");
  n=0;
  for (i=0; i<anz->e; i++)
  {
    n++;
    fprintf (handle1, " %d", elem[i].nr);
    if(n>6) { n=0; fprintf (handle1, "\n"); }
  }
  fprintf (handle1, "\n");
  fclose(handle1);
}



void senddata( char *setname, char *format, Summen *anz, Nodes *node, Elements *e_enqire, Datasets *lcase , Scale *scale, int compFlag, char **val)
{
  int   setNr;
  int  length, i, j, n, ipuf=0, lc,lct, comp;
  char prognam[MAX_LINE_LENGTH];
  Summen    anz2[1];
  Nodes     *node2;
  Elements  *elem2;
  Datasets *lcase2=NULL;
  static int   *isort=NULL;
  extern int compareInt();
  static int step_number2=0;
  FILE *handle;
  double dx,dy,dz, sum_l=0.;
  int  xl[26];
  int *e2=NULL;
#define MM_TO_M  0.001
#define DT_KELVIN_TO_C -273.15

  setNr=getSetNr(setname);
  if (setNr<0)
  {
    printf (" ERROR in senddata, set:%s does not exist\n", setname);
    return;
  }
  /* replace 'lc' by 'ds' */
  if(val!=NULL) for(i=0; i<2; i++) if(compare( val[i], "lc", 2)== 2) { val[i][0]='d'; val[i][1]='s'; }
  strcpy ( prognam, setname);

  /* set is a sequence-set, tabular format */
  if(set[setNr].type)
  {
    length= strlen ( setname );
    strcpy (&prognam[length], ".txt");
    handle = fopen (prognam, "w+b" );
    if (handle==NULL) { printf ("\nThe output file \"%s\" could not be opened.\n\n", prognam); return;}

    if (compare( format, "seq", 3) == 3)
    {
      for (n=0; n<set[setNr].anz_n; n++)
      {
        fprintf ( handle, "%d\n",set[setNr].node[n]);
      }
      fclose(handle);
    }

    /* 1D stress for darwin in tabular format */
    if (compare( format, "dar", 3) == 3)
    {
      fprintf (handle, "# X               sigma\n");
      fprintf ( handle, "%-12.5e %-12.5e\n", sum_l* scale->w  * MM_TO_M, lcase[cur_lc].dat[cur_entity][set[setNr].node[0]]);
      for (n=1; n<set[setNr].anz_n; n++)
      {
        dx=node[set[setNr].node[n]].nx - node[set[setNr].node[n-1]].nx;
        dy=node[set[setNr].node[n]].ny - node[set[setNr].node[n-1]].ny;
        dz=node[set[setNr].node[n]].nz - node[set[setNr].node[n-1]].nz;
        sum_l+=sqrt(dx*dx+dy*dy+dz*dz);

        fprintf ( handle, "%-12.5e %-12.5e\n", (sum_l* scale->w) * MM_TO_M, lcase[cur_lc].dat[cur_entity][set[setNr].node[n]] );
      }
      fclose(handle);
    }
    return;
  }

  /* check if the format is known, else return */
  if ( (compare( format, "frd", 3) == 3)||
       (compare( format, "nas", 3) == 3)||
       (compare( format, "tcg", 3) == 3)||
       (compare( format, "lst", 3) == 3)||
       (compare( format, "abq", 3) == 3)||
       (compare( format, "ans", 3) == 3)||
       (compare( format, "ast", 3) == 3)||
       (compare( format, "dar", 3) == 3)||
       (compare( format, "sam", 3) == 3)||
       (compare( format, "stl", 3) == 3) );
  else
  {
    printf (" ERROR: format not recognized:%s\n\n", format);
    return;
  }

  descalNodes ( anz->n, node, scale);

  /* if the parameter sur was specified, send only the faces (with dataset) */
  if((val!=NULL)&&(compare( val[0], "sur", 3)== 3))
  {
    if((val!=NULL)&&(compare( val[1], "ds", 2)== 2)) strcpy(val[0],val[1]);
    anz2->emax=0;
    anz2->emin=MAX_INTEGER;
    anz2->e = set[setNr].anz_f;
    for (i=0; i<13; i++)
      anz2->etype[i] = 0;

    if ( (elem2 = (Elements *)malloc( (set[setNr].anz_f+1) * sizeof(Elements))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }
    if ( (isort = (int *)realloc( isort, (set[setNr].anz_f+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed: isort\n\n" ); 
    for( i=0; i<set[setNr].anz_f; i++) isort[i]=set[setNr].face[i];
    qsort( isort, set[setNr].anz_f, sizeof(int), (void *)compareInt );

    for (i=0; i<set[setNr].anz_f; i++)
    {
      elem2[i].nr      = isort[i]+1;
      elem2[i].type    = face[isort[i]].type;
      elem2[i].group   = face[isort[i]].group;
      elem2[i].mat     = face[isort[i]].mat;
      elem2[i].attr    = 0;
      anz2->etype[elem2[i].type]++;
      
      if (anz2->emax<elem2[i].nr ) anz2->emax = elem2[i].nr;
      if (anz2->emin>elem2[i].nr ) anz2->emin = elem2[i].nr;
      ipuf=0;
      if (elem2[i].type == 7) ipuf = 3;  /* TRI3  */
      if (elem2[i].type == 8) ipuf = 6;  /* TRI6  */
      if (elem2[i].type == 9) ipuf = 4;  /* QUAD4 */
      if (elem2[i].type == 10) ipuf = 8; /* QUAD8 */
      if (elem2[i].type == 11) ipuf = 2; /* BEAM */
      if (elem2[i].type == 12) ipuf = 3; /* BEAM3 */
      if (ipuf==0)
      {
        if(printFlag) printf (" elem(%d) not a known type (%d)\n", elem2[i].nr, elem2[i].type);
      }
      else
      {
        if (compFlag)
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[i].nod[j]=face[isort[i]].nod[j];
            seta( setNr, "n", elem2[i].nod[j] );
          }
        }
        else
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[i].nod[j]=face[isort[i]].nod[j];
          }
        }
      }
    }
  }
  /* if the parameter sur was not specified, send the elements (with dataset) */
  else
  {
    if ( (elem2 = (Elements *)malloc( (set[setNr].anz_e+1) * sizeof(Elements))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }
    if ( (isort = (int *)realloc( isort, (set[setNr].anz_e+1) * sizeof(int))) == NULL )
      printf("ERROR: realloc failed: isort\n\n" ); 
    for( i=0; i<set[setNr].anz_e; i++) isort[i]=set[setNr].elem[i];
    qsort( isort, set[setNr].anz_e, sizeof(int), (void *)compareInt );

    anz2->emax=0;
    anz2->emin=MAX_INTEGER;
    anz2->e = 0;
    elem2[0].nr =0;
    for (i=0; i<set[setNr].anz_e; i++)
    {
      if((i)&&(isort[i]==isort[i-1])) continue;

      elem2[anz2->e].nr      = isort[i];
      elem2[anz2->e].type    = e_enqire[isort[i]].type;
      elem2[anz2->e].group   = e_enqire[isort[i]].group;
      elem2[anz2->e].mat     = e_enqire[isort[i]].mat;
      elem2[anz2->e].attr    = e_enqire[isort[i]].attr;
      
      if (anz2->emax<elem2[anz2->e].nr ) anz2->emax = elem2[anz2->e].nr;
      if (anz2->emin>elem2[anz2->e].nr ) anz2->emin = elem2[anz2->e].nr;
      ipuf=0;
      if (elem2[anz2->e].type == 1) ipuf = 8;  /* HEXA8 */
      if (elem2[anz2->e].type == 2) ipuf = 6;  /* PENTA6 */
      if (elem2[anz2->e].type == 3) ipuf = 4;  /* TET4 */
      if (elem2[anz2->e].type == 4) ipuf = 20; /* HEX20 */
      if (elem2[anz2->e].type == 5) ipuf = 15; /* PENTA15 */
      if (elem2[anz2->e].type == 6) ipuf = 10; /* TET10 */
      if (elem2[anz2->e].type == 7) ipuf = 3;  /* TRI3  */
      if (elem2[anz2->e].type == 8) ipuf = 6;  /* TRI6  */
      if (elem2[anz2->e].type == 9) ipuf = 4;  /* QUAD4 */
      if (elem2[anz2->e].type == 10) ipuf = 8; /* QUAD8 */
      if (elem2[anz2->e].type == 11) ipuf = 2; /* BEAM */
      if (elem2[anz2->e].type == 12) ipuf = 3; /* BEAM3 */
      if (ipuf==0)
      {
        if(printFlag) printf (" elem(%d) not a known type (%d)\n", elem2[anz2->e].nr, elem2[anz2->e].type);
      }
      else
      {
        if (compFlag)
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[anz2->e].nod[j]=e_enqire[elem2[anz2->e].nr].nod[j];
            seta( setNr, "n", elem2[anz2->e].nod[j] );
          }
        }
        else
        {
          for (j=0; j<ipuf; j++)
          {
            elem2[anz2->e].nod[j]=e_enqire[elem2[anz2->e].nr].nod[j];
          }
        }
      }
      /* if parameter frdm change penta to hexa */
      if ( (format[3]=='m')&&(elem2[anz2->e].type==5) )
      {
        printf (" elem(%d) of type (%d) will be changed to hex20\n", elem2[anz2->e].nr, elem2[anz2->e].type);

        for(j=0; j<3; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j];
        }
        xl[j]= xl[j-1];
        for(j=4; j<7; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-1];
        }
        xl[j]= xl[j-1];
        for(j=8; j<10; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-2];
        }
        xl[j++]= elem2[anz2->e].nod[2];
        xl[j]= elem2[anz2->e].nod[8];
  
        for(j=12; j<14; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j];
        }
         xl[j++]= elem2[anz2->e].nod[5];
        xl[j]= elem2[anz2->e].nod[14];
  
        for(j=16; j<19; j++)
        { 
          xl[j]= elem2[anz2->e].nod[j-7];
        }
        xl[j]= xl[j-1];
  
  
        for(j=0; j<12; j++)
        { 
           elem2[anz2->e].nod[j]=xl[j];
        }
        for(n=16; n<20; n++)
        { 
           elem2[anz2->e].nod[n]=xl[j];
          j++;
        }
        for(n=12; n<16; n++)
        { 
           elem2[anz2->e].nod[n]=xl[j];
          j++;
        }

	elem2[anz2->e].type=4;
      }
      anz2->etype[elem2[anz2->e].type]++;
      anz2->e++;
    }
  }

  /* nodes must follow the elements (seta n above!) */
  /* sort the nodenumbers */

  if ( (node2 = (Nodes *)malloc( (anz->nmax+1) * sizeof(Nodes))) == NULL )
    { printf(" ERROR: malloc failed in senddata\n\n") ; return; }

  if ( (isort = (int *)realloc( isort, (set[setNr].anz_n+1) * sizeof(int))) == NULL )
    printf("ERROR: realloc failed: isort\n\n" ); 
  for( i=0; i<set[setNr].anz_n; i++) isort[i]=set[setNr].node[i];
  qsort( isort, set[setNr].anz_n, sizeof(int), (void *)compareInt  );

  anz2->nmax = 0;
  anz2->nmin = MAX_INTEGER;
  anz2->n = 0;;
  node2[0].nr =0;
  for (i=0; i<set[setNr].anz_n; i++)
  {
    if((i)&&(isort[i]==isort[i-1])) continue;

    node2[anz2->n].nr = isort[i];
    node2[isort[i]].nx = node[isort[i]].nx;
    node2[isort[i]].ny = node[isort[i]].ny;
    node2[isort[i]].nz = node[isort[i]].nz;
    if (anz2->nmax<node2[anz2->n].nr) anz2->nmax = node2[anz2->n].nr;
    if (anz2->nmin>node2[anz2->n].nr) anz2->nmin = node2[anz2->n].nr;
    anz2->n++;
  }

  strcpy( anz2->model, setname);
  anz2->l = anz->l;
  anz2->u = anz->u;
  anz2->p = anz->p;
  anz2->sets = anz->sets;
  anz2->uheader = anz->uheader;
  anz2->pheader = anz->pheader;

  if(printFlag) printf (" write file \n");
  length= strlen ( setname );

  if (compare( format, "lst", 3) == 3)
  {
    strcpy (&prognam[length], ".lst");
    write2lst( prognam, anz2, node2, elem2);
  }
  else if (compare( format, "frd", 3) == 3)
  {
    strcpy (&prognam[length], ".frd");

    /* prepare the Datasets only if ds is specified */
    if((val)&&((compare( val[0], "ds", 2)== 2)||(compare( val[0], "bin", 2)== 2)))
    {
      if(val[0][strlen(val[0])-1]=='+') ipuf=2; else ipuf=0;
      if(compare( val[0], "ds", 2)== 2)
      {
        if(val[0][2]=='l') lc=anz->l;
        else lc=atoi(&val[0][2]);
      }
      else
      {
        if(val[0][3]=='l') lc=anz->l;
        else lc=atoi(&val[0][3]);
      }
      if(lc<0) lc=anz->l+lc;

      if(lc)
      {
        lc--;
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
        anz2->l = 1;
        if ( (lcase2 = (Datasets *)realloc((Datasets *)lcase2, 1 * sizeof(Datasets))) == NULL )
        { printf("\n\n ERROR: malloc failure\n\n" ); exit(1); }

        strcpy(lcase2[0].name, lcase[lc].name);
        lcase2[0].value= lcase[lc].value ;
        lcase2[0].npheader = lcase[lc].npheader;
        lcase2[0].irtype = lcase[lc].irtype;
        lcase2[0].step_number=++step_number2;
        lcase2[0].analysis_type=lcase[lc].analysis_type;
        strcpy(lcase2[0].dataset_name,lcase[lc].dataset_name );
        strcpy(lcase2[0].dataset_text,lcase[lc].dataset_text);
        strcpy(lcase2[0].analysis_name,lcase[lc].analysis_name);
        lcase2[0].pheader=lcase[lc].pheader;
        lcase2[0].ncomps=0;
        lcase2[0].menu=NULL;
        lcase2[0].ictype=NULL;
        lcase2[0].icind1=NULL;
        lcase2[0].icind2=NULL;
        lcase2[0].iexist=NULL;
        lcase2[0].max=NULL;         
        lcase2[0].min=NULL;         
        lcase2[0].nmax=NULL;        
        lcase2[0].nmin=NULL;        
        lcase2[0].dat=NULL;       
        lcase2[0].compName=NULL;
        lcase2[0].icname=NULL;

        if ( (e2 = (int *)realloc(e2,  (lcase[lc].ncomps+1) * sizeof(int))) == NULL )
          printf("\n\n ERROR: malloc failed\n\n") ;
        if(val[1][0]=='e')
        {
          for(i=0; i<lcase[lc].ncomps; i++) e2[i]=0;
          lcparser(&val[1][1], e2);
	}
        else
          for(i=0; i<lcase[lc].ncomps; i++) e2[i]=1;

        for(i=0; i<lcase[lc].ncomps; i++)
        {
          if(e2[i]==1)
          {
            lcase2[0].ncomps++;

            if ( (lcase2[0].nmax = (int *)realloc( (int *)lcase2[0].nmax, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].nmin = (int *)realloc( (int *)lcase2[0].nmin, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].max = (float *)realloc((float *)lcase2[0].max, lcase2[0].ncomps * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].min = (float *)realloc((float *)lcase2[0].min, lcase2[0].ncomps * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].dat = (float **)realloc((float **)lcase2[0].dat, lcase2[0].ncomps * sizeof(float *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].compName = (char **)realloc((char **)lcase2[0].compName, lcase2[0].ncomps * sizeof(char *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icname = (char **)realloc((char **)lcase2[0].icname, lcase2[0].ncomps * sizeof(char *))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].menu = (int *)realloc((int *)lcase2[0].menu, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].ictype = (int *)realloc((int *)lcase2[0].ictype, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icind1 = (int *)realloc((int *)lcase2[0].icind1, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].icind2 = (int *)realloc((int *)lcase2[0].icind2, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
            if ( (lcase2[0].iexist = (int *)realloc((int *)lcase2[0].iexist, lcase2[0].ncomps * sizeof(int))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );
          
            comp=lcase2[0].ncomps-1;
            if ( (lcase2[0].dat[comp] = (float *)malloc( (anz2->nmax+1) * sizeof(float))) == NULL )
              printf("\n\n ERROR: malloc failure\n\n" );	               
            if ( (lcase2[0].compName[comp] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
              printf("\n\n ERROR: malloc failed\n\n" );
            if ( (lcase2[0].icname[comp] = (char *)malloc( MAX_LINE_LENGTH * sizeof(char))) == NULL )
              printf("\n\n ERROR: malloc failed\n\n" );

            lcase2[0].max[comp]=lcase[lc].max[i];
            lcase2[0].min[comp]=lcase[lc].min[i];
            strcpy ( lcase2[0].compName[comp], lcase[lc].compName[i]);
            lcase2[0].menu[comp]   = lcase[lc].menu[i]  ;
            lcase2[0].ictype[comp] = lcase[lc].ictype[i];
            lcase2[0].icind1[comp] = lcase[lc].icind1[i];
            lcase2[0].icind2[comp] = lcase[lc].icind2[i];
            lcase2[0].iexist[comp] = lcase[lc].iexist[i];
            for(n=0; n<=anz2->nmax; n++) lcase2[0].dat[comp][n]=lcase[lc].dat[i][n];
          }
	}
        write2frd( prognam, anz2, node2, elem2, lcase2, ipuf);

        if(lcase2[0].ncomps)
	{
          for(i=0; i<lcase2[0].ncomps; i++)
          {
            free( lcase2[0].icname[i]); free( lcase2[0].compName[i]); free(lcase2[0].dat[i]);
          }
          free( lcase2[0].menu);
          free( lcase2[0].ictype);
          free( lcase2[0].icind1);
          free( lcase2[0].icind2);
          free( lcase2[0].iexist);
          free( lcase2[0].max);         
          free( lcase2[0].min);         
          free( lcase2[0].nmax);        
          free( lcase2[0].nmin);        
          free( lcase2[0].dat);       
          free( lcase2[0].compName);
          free( lcase2[0].icname);
	}
      }
      else
      {
       for(lc=0; lc<anz->l; lc++)
       {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
       }
       if(compare( val[0], "bin", 2)== 2) write2frd( prognam, anz2, node2, elem2, lcase, 1);
       else  write2frd( prognam, anz2, node2, elem2, lcase, ipuf);
      }
    }
    else
    {
      anz2->l = 0;
      write2frd( prognam, anz2, node2, elem2, lcase, 0);
    } 
  }
  else if (compare( format, "nas", 3) == 3)
  {
    /* prepare the Datasets only if ds is specified */
    if((val)&&(compare( val[0], "ds", 2)== 2))
    {
      for(lc=0; lc<anz->l; lc++)
      {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
      }
    }
    else     anz2->l = 0;

    /* in case a coordinate system for nodes regarding the DOFs was defined */
    for (i=0; i<anz2->n; i++) node2[node2[i].nr].pflag=0;
    if(nodeCsys)
    {
      for(j=0; j<nodeCsys; j++)
      {
        printf(" nodes of set:%s get csys:%d\n", set[nodeCsysSet[j]].name, nodeCsysNr[j]);
        for (i=0; i<set[nodeCsysSet[j]].anz_n; i++) node2[set[nodeCsysSet[j]].node[i]].pflag=nodeCsysNr[j];
      }
    }
    write2nas( prognam, anz2, node2, elem2, lcase);
  }
  else if (compare( format, "abq", 3) == 3)
  {
    if(val)
    {
     if(compare( val[0], "ds", 2)== 2)
     {
       lc=atoi(&val[0][2])-1;
       if(lc==-1)
       {
         printf("\n ERROR: No dataset nr was given, only one dataset at a time can be written.\n\n");
	 return;
       }
       /* check if the data of the specified lcase (Dataset) are already available */
       if (!lcase[lc].loaded)
       {
         if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
         {
           printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
           return;
         }
         calcDatasets( lc, anz, node, lcase );
         recompileEntitiesInMenu(lc);
       }
       /* check if the data of the specified lcase (Dataset) are already available */
       if (!lcase[lc].loaded)
       {
         if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
         {
           printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
           return;
         }
         calcDatasets( lc, anz, node, lcase );
         recompileEntitiesInMenu(lc);
       }
     }
     if((compare( val[0], "tmf", 3)== 3)||(compare( val[0], "sta", 3)== 3)||(compare( val[0], "crp", 3)== 3))
     {
      for(lc=0; lc<anz->l; lc++)
      {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
      }
     }
    }

    /* write the equations for incompatible elements */
    for(i=0; i<sum_equSets; i++)
    {
      /* temporary change the element-type to exclude unwanted elements */
      for(j=0; j<anz->e; j++) e_enqire[e_enqire[j].nr].type+=100;
      for(j=0; j<set[indSet[i]].anz_e; j++) e_enqire[set[indSet[i]].elem[j]].type-=100;

      /* gen mpcs */
      rnam(depSet[i], &set[depSet[i]].name[1]);
      areampc(depSet[i], indSet[i], "abq", "areampc", "123", "c", 0, node, 1);

      /* restore orig type */
      for(j=0; j<set[indSet[i]].anz_e; j++) e_enqire[set[indSet[i]].elem[j]].type+=100;
      for(j=0; j<anz->e; j++) e_enqire[e_enqire[j].nr].type-=100;
    } 

    write2aba( prognam, anz2, node2, elem2, lcase, val);

    /* add the equations to the mesh-file */
    for(i=0; i<sum_equSets; i++)
    {
      sprintf(buffer,"cat %s.equ >> %s", set[depSet[i]].name, prognam);
      system(buffer);    
      sprintf(buffer,"rm -f %s.equ", set[depSet[i]].name);
      system(buffer);
      sprintf(buffer,"-%s", set[depSet[i]].name);
      rnam(depSet[i], buffer);
    }
  }
  else if (compare( format, "dar", 3) == 3)
  {
    /* prepare the Datasets only if ds is specified */
    if((val)&&(val[0][0]=='v')); 
    else if((val)&&(compare( val[0], "ds", 2)== 2))
    {
      lc=atoi(&val[0][2]);
      if(lc)
      {
        lc--;

        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }

        /* search for related temps */      
        //if(lc>0) { lct=lc-1; while((lcase[lct].step_number==lcase[lc].step_number)&&(lct>0)) lct--; lct++; }
        if(lc) { for (lct=lc-1; lct>=0; lct--) { if(lcase[lct].step_number!=lcase[lc].step_number) break; } lct++; }
        else lct=1;
        while((lct<anz->l)&&(lcase[lct].step_number==lcase[lc].step_number))
        {
          if( (compare( lcase[lct].name, "NDTEMP", 6) == 6)||( compare( lcase[lct].name, "TEMP", 4) == 4)||( compare( lcase[lct].name, "TT3D", 4) == 4))
          {
            if (!lcase[lc].loaded)
            {
              if( pre_readfrdblock(copiedNodeSets , lct, anz, node, lcase )==-1) 
              {
                printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lct+1); 
                return;
              }
              calcDatasets( lct, anz, node, lcase );
              recompileEntitiesInMenu(lct);
            }
          }
          lct++;
        }
      }
      else
      {
       for(lc=0; lc<anz->l; lc++)
       {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf(" ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
       }
      }
    }
    else val=0;
    write2darwin( prognam, anz2, node2, elem2, lcase, val);
  }
  else if (compare( format, "ans", 3) == 3)
  {
    /* prepare the Datasets only if lc is specified */
    if((val)&&(compare( val[0], "ds", 2)== 2))
    {
      for(lc=0; lc<anz->l; lc++)
      {
        /* check if the data of the specified lcase (Dataset) are already available */
        if (!lcase[lc].loaded)
        {
          if( pre_readfrdblock(copiedNodeSets , lc, anz, node, lcase )==-1) 
          {
            printf("ERROR in nodalDataset: Could not read data for Dataset:%d\n", lc+1); 
            return;
          }
          calcDatasets( lc, anz, node, lcase );
          recompileEntitiesInMenu(lc);
        }
      }
    }
    else     anz2->l = 0;
    write2ansys( prognam, anz2, node2, elem2, lcase);
  }
  else if (compare( format, "ast", 3) == 3)
  {
    anz2->l = 0;
    write2aster( prognam, anz2, node2, elem2, set, lcase);  //TODD
  }
  else if (compare( format, "sam", 3) == 3)
  {
    anz2->l = 0;
    write2samcef( prognam, anz2, node2, elem2, lcase);
  }
  else if (compare( format, "tcg", 3) == 3)
  {
    anz2->l = 0;
    write2tochnog( prognam, anz2, node2, elem2, lcase);
  }
  else
    printf (" ERROR: Format not recognized");

  free(node2);
  free(elem2);
  scalNodes ( anz->n, node, scale );
}



void setLightAndMaterial_rgb(int col)
{
  GLfloat mat_shininess[1]; /* 0->128 Blankheit */
  GLfloat mat_specular[4]; /* ungerichtete reflektion*/
  GLfloat mat_diffuse[4];  /* gerichtete reflektion*/

  mat_shininess[0]= 128.;
  mat_specular[0]=MAT_SPEC;
  mat_specular[1]=MAT_SPEC;
  mat_specular[2]=MAT_SPEC;
  mat_specular[3]=1.0;          

  mat_diffuse[0]=entity_r[col];
  mat_diffuse[1]=entity_g[col];
  mat_diffuse[2]=entity_b[col];
  mat_diffuse[3]=1.0;          

  /* Reflexionseigenschaften des Materials aufbringen */
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_specular);
}


