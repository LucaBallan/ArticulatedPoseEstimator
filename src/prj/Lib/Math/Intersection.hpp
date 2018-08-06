/* Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 *
 * Updated June 1999: removed the divisions -- a little faster now!
 * Updated October 1999: added {} to ITST_CROSS and ITST_SUB macros 
 *
 * int NoDivTriTriIsect(Element_Type V0[3],Element_Type V1[3],Element_Type V2[3],
 *                      Element_Type U0[3],Element_Type U1[3],Element_Type U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns true if the triangles intersect, otherwise false
 *
 */





/* some macros */
#define ITST_CROSS(dest,v1,v2){                     \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1];      \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2];      \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];}

#define ITST_DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define ITST_SUB(dest,v1,v2){         \
            dest[0]=v1[0]-v2[0];      \
            dest[1]=v1[1]-v2[1];      \
            dest[2]=v1[2]-v2[2];}


/* sort so that a<=b */
#define ITST_SORT(a,b)             \
             if(a>b)               \
             {                     \
               Element_Type c; \
               c=a;                \
               a=b;                \
               b=c;                \
             }


/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return true;                   \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return true;                   \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  Element_Type Ax,Ay,Bx,By,Cx,Cy,e,d,f;    \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  Element_Type a,b,c,d0,d1,d2;          \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return true;              \
  }                                         \
}


inline bool coplanar_tri_tri(Element_Type N[3],Element_Type V0[3],Element_Type V1[3],Element_Type V2[3],
                     Element_Type U0[3],Element_Type U1[3],Element_Type U2[3])
{
   Element_Type A[3];
   int i0,i1;
   /* first project onto an axis-aligned plane, that maximizes the area */
   /* of the triangles, compute indices: i0,i1. */
   A[0]=fabs(N[0]);
   A[1]=fabs(N[1]);
   A[2]=fabs(N[2]);
   if(A[0]>A[1])
   {
      if(A[0]>A[2])
      {
          i0=1;      /* A[0] is greatest */
          i1=2;
      }
      else
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
      else
      {
          i0=0;      /* A[1] is greatest */
          i1=2;
      }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0,U0,U1,U2);
    POINT_IN_TRI(U0,V0,V1,V2);

    return false;
}



#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
                /* here we know that d0d1<=0.0 */ \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                /* triangles are coplanar */ \
                return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
        } \
}

inline bool NoDivTriTriIsect(Element_Type V0[3],Element_Type V1[3],Element_Type V2[3],
						     Element_Type U0[3],Element_Type U1[3],Element_Type U2[3])
{
  Element_Type E1[3],E2[3];
  Element_Type N1[3],N2[3],d1,d2;
  Element_Type du0,du1,du2,dv0,dv1,dv2;
  Element_Type D[3];
  Element_Type isect1[2], isect2[2];
  Element_Type du0du1,du0du2,dv0dv1,dv0dv2;
  int index;
  Element_Type vp0,vp1,vp2;
  Element_Type up0,up1,up2;
  Element_Type bb,cc,max;

  /* compute plane equation of triangle(V0,V1,V2) */
  ITST_SUB(E1,V1,V0);
  ITST_SUB(E2,V2,V0);
  ITST_CROSS(N1,E1,E2);
  d1=-ITST_DOT(N1,V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
  du0=ITST_DOT(N1,U0)+d1;
  du1=ITST_DOT(N1,U1)+d1;
  du2=ITST_DOT(N1,U2)+d1;

  /* coplanarity robustness check */
  if(fabs(du0)<INTERSECTION_NUMERICAL_TOLLERANCE) du0=0.0;
  if(fabs(du1)<INTERSECTION_NUMERICAL_TOLLERANCE) du1=0.0;
  if(fabs(du2)<INTERSECTION_NUMERICAL_TOLLERANCE) du2=0.0;
  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  ITST_SUB(E1,U1,U0);
  ITST_SUB(E2,U2,U0);
  ITST_CROSS(N2,E1,E2);
  d2=-ITST_DOT(N2,U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0=ITST_DOT(N2,V0)+d2;
  dv1=ITST_DOT(N2,V1)+d2;
  dv2=ITST_DOT(N2,V2)+d2;

  if(fabs(dv0)<INTERSECTION_NUMERICAL_TOLLERANCE) dv0=0.0;
  if(fabs(dv1)<INTERSECTION_NUMERICAL_TOLLERANCE) dv1=0.0;
  if(fabs(dv2)<INTERSECTION_NUMERICAL_TOLLERANCE) dv2=0.0;

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;

  if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
    return false;				 /* no intersection occurs */

  /* compute direction of intersection line */
  ITST_CROSS(D,N1,N2);

  /* compute and index to the largest component of D */
  max=(Element_Type)fabs(D[0]);
  index=0;
  bb=(Element_Type)fabs(D[1]);
  cc=(Element_Type)fabs(D[2]);
  if(bb>max) max=bb,index=1;
  if(cc>max) max=cc,index=2;

  /* this is the simplified projection onto L*/
  vp0=V0[index];
  vp1=V1[index];
  vp2=V2[index];

  up0=U0[index];
  up1=U1[index];
  up2=U2[index];

  /* compute interval for triangle 1 */
  Element_Type a,b,c,x0,x1;
  NEWCOMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,a,b,c,x0,x1);

  /* compute interval for triangle 2 */
  Element_Type d,e,f,y0,y1;
  NEWCOMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,d,e,f,y0,y1);

  Element_Type xx,yy,xxyy,tmp;
  xx=x0*x1;
  yy=y0*y1;
  xxyy=xx*yy;

  tmp=a*xxyy;
  isect1[0]=tmp+b*x1*yy;
  isect1[1]=tmp+c*x0*yy;

  tmp=d*xxyy;
  isect2[0]=tmp+e*xx*y1;
  isect2[1]=tmp+f*xx*y0;

  ITST_SORT(isect1[0],isect1[1]);
  ITST_SORT(isect2[0],isect2[1]);

  if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) return false;
  return true;
}
