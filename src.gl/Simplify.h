class SymetricMatrix { 

	public:

	// Constructor
	
	SymetricMatrix(double c=0) { loopi(0,10) m[i] = c;  }

	//4*4�Գƾ����������
	SymetricMatrix(	double m11, double m12, double m13, double m14, 
			            double m22, double m23, double m24,
			                        double m33, double m34,
			                                    double m44) {
			 m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14; 
			              m[4] = m22;  m[5] = m23;  m[6] = m24; 
			                           m[7] = m33;  m[8] = m34;
			                                        m[9] = m44;
	}

	// Make plane
	//�����������������(a,b,c,d��ƽ�淽�̵�ϵ��:ax+by+cz+d=0)
	SymetricMatrix(double a,double b,double c,double d)
	{
		m[0] = a*a;  m[1] = a*b;  m[2] = a*c;  m[3] = a*d; 
		             m[4] = b*b;  m[5] = b*c;  m[6] = b*d; 
		                          m[7 ] =c*c; m[8 ] = c*d;
		                                       m[9 ] = d*d;
	}
	
	double operator[](int c) const { return m[c]; }

	// Determinant
	//3*3��������ʽ(���Խ��߳˻�֮�ͼ�ȥ���Խ��߳˻�֮��)
	double det(	int a11, int a12, int a13,
				int a21, int a22, int a23,
				int a31, int a32, int a33)
	{
		double det =  m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31] 
					- m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32]- m[a12]*m[a21]*m[a33]; 
		return det;
	}

	//�������
	const SymetricMatrix operator+(const SymetricMatrix& n) const
	{ 
		return SymetricMatrix( m[0]+n[0],   m[1]+n[1],   m[2]+n[2],   m[3]+n[3], 
						                    m[4]+n[4],   m[5]+n[5],   m[6]+n[6], 
						                                 m[ 7]+n[ 7], m[ 8]+n[8 ],
						                                              m[ 9]+n[9 ]);
	}

	//�������
	SymetricMatrix& operator+=(const SymetricMatrix& n)
	{
		 m[0]+=n[0];   m[1]+=n[1];   m[2]+=n[2];   m[3]+=n[3]; 
		 m[4]+=n[4];   m[5]+=n[5];   m[6]+=n[6];   m[7]+=n[7]; 
		 m[8]+=n[8];   m[9]+=n[9];
		return *this; 
	}

	double m[10];
};
///////////////////////////////////////////

namespace Simplify
{
	// Global Variables & Strctures

	struct Triangle { int v[3];double err[4];int deleted,dirty;vec3f n; };
	struct Vertex { vec3f p;int tstart,tcount;SymetricMatrix q;int border;};
	struct Ref { int tid,tvertex; }; 
	std::vector<Triangle> triangles;
	std::vector<Vertex> vertices;
	std::vector<Ref> refs;
	
	// Helper functions

	double vertex_error(SymetricMatrix q, double x, double y, double z);
	double calculate_error(int id_v1, int id_v2, vec3f &p_result);
	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted);
	void update_triangles(int i0,Vertex &v,const std::vector<int> &deleted,int &deleted_triangles);
	void update_mesh(int iteration);
	void compact_mesh();
	//
	// Main simplification function 
	//
	// target_count  : target nr. of triangles
	// agressiveness : sharpness to increase the threashold.
	//                 5..8 are good numbers
	//                 more iterations yield higher quality
	//
	void simplify_mesh(int target_count, double agressiveness=7)
	{
		// init
		printf("%s - start\n",__FUNCTION__);
		int timeStart=timeGetTime();

		loopi(0,triangles.size()) triangles[i].deleted=0;  //ɾ�����
		
		// main iteration loop 

		int deleted_triangles=0; 
		std::vector<int> deleted0,deleted1;
		int triangle_count=triangles.size();
		
		loop(iteration,0,100) 
		{
			// target number of triangles reached ? Then break
			printf("iteration %d - triangles %d\n",iteration,triangle_count-deleted_triangles);
			if(triangle_count-deleted_triangles<=target_count)break;

			// update mesh once in a while
			if(iteration%5==0) 
			{
				/*
				1)ɾ��delete��������(ÿ�ε���)
				2)��ʼ���ߵ�����ֵ(���ε���)
				3)���¼����������ӹ�ϵ(ÿ�ε���)
				4)�ռ��߽����Ϣ(���ε���)
				*/
				update_mesh(iteration);
			}

			// clear dirty flag
			loopi(0,triangles.size()) triangles[i].dirty=0;  //�Ƿ���Ҫ���¼�������ֵ�ı��
			
			//
			// All triangles with edges below the threshold will be removed
			//
			// The following numbers works well for most models.
			// If it does not, try to adjust the 3 parameters
			//
			double threshold = 0.000000001*pow(double(iteration+3),agressiveness);

			// remove vertices & mark deleted triangles			
			loopi(0,triangles.size())
			{				
				Triangle &t=triangles[i];
				if(t.err[3]>threshold) continue;  //������������ֵ��С���Ѿ�������ֵ
				if(t.deleted) continue;  //���������ߵ������α��Ϊɾ��״̬
				if(t.dirty) continue;    //�������������ߵ�����һ������������α��Ϊ����(��Ҫ���¼�������ֵ)״̬
				
				loopj(0,3)if(t.err[j]<threshold) 
				{
					int i0=t.v[ j     ]; Vertex &v0 = vertices[i0]; 
					int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];

					// Border check
					if(v0.border != v1.border)  continue;  //�ߵ������˵�����һ����ֻ��һ������������

					// Compute vertex to collapse to
					vec3f p;
					calculate_error(i0,i1,p);  //�����������ߵ�

					deleted0.resize(v0.tcount); // normals temporarily v0���ӵ�����������
					deleted1.resize(v1.tcount); // normals temporarily v1���ӵ�����������

					/*
					1)��⽫Ҫ�仯(delta)���������õ�p�������е�һ���˵��Ƿ�ᷢ����ת(����ƫת����78.5��);
					2)�ռ������߹�������Ҫ���ı����ӵ�������(dead),��deleted0,deleted1��Ӧ��λ�������1;
					*/
					if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;
					if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;

					// not flipped, so remove edge
					/*
					1)ɾ��dead������(���{i0,i1}���ӵ�����������);
					2)��ʧһ������vertex(v1);
					*/
					v0.p=p;  //����v0,�����Ż���(���������С��)����v0�ļ���λ��;
					v0.q=v1.q+v0.q; //��v1�Ķ���������(ƽ�淽��)�ۼӵ�v0��;
					int tstart=refs.size();

					/*
					1)��delta�����ε�v0����v1�����ڸ����������еĽ�ɫ�滻��i0;
					2)���¼���delta�����θ��ߵĶ����������ŵ�;
					3)��������corner����׷�ӵ�refs.back();
					*/
					update_triangles(i0,v0,deleted0,deleted_triangles);
					update_triangles(i0,v1,deleted1,deleted_triangles);
					
					//�������߲������µ�corner����
					int tcount=refs.size()-tstart;
				
					//v0��corner�ռ��㹻������corner����,��ֱ�ӿ���
					if(tcount<=v0.tcount)
					{
						// save ram
						if(tcount)memcpy(&refs[v0.tstart],&refs[tstart],tcount*sizeof(Ref));
					}
					//v0��corner�ռ䲻��,������v0��corner�ռ��v0.tstart,ָ���µ�λ��
					else
						// append
						v0.tstart=tstart;

					//�����ƶ�v0��corner�������Ŀ
					v0.tcount=tcount;
					break;
				}
				// done?
				//������ֹ�ж�
				if(triangle_count-deleted_triangles<=target_count)break;
			}
		}

		// clean up mesh
		//ɾ�������������κͶ������,����ȷ��corner��Ϣ����
		compact_mesh();

		// ready
		//�������
		int timeEnd=timeGetTime();
		printf("%s - %d/%d %d%% removed in %d ms\n",__FUNCTION__,
			triangle_count-deleted_triangles,
			triangle_count,deleted_triangles*100/triangle_count,
			timeEnd-timeStart);
		
	}

	// Check if a triangle flips when this edge is removed
	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted)
	{
		int bordercount=0;

		//����p�Ƿ���v0���ӵ��������г�v0������������㹹�ɷ���
		loopk(0,v0.tcount)
		{
			Triangle &t=triangles[refs[v0.tstart+k].tid]; 
			if(t.deleted)continue; //�Ѿ����Ϊɾ��

			int s=refs[v0.tstart+k].tvertex;
			int id1=t.v[(s+1)%3];
			int id2=t.v[(s+2)%3];

			//������t���{i0,i1}����,��t���ڱ�{i0,i1}���߲����б�ɾ��
			if(id1==i1 || id2==i1) // delete ?
			{
				bordercount++;
				deleted[k]=1;
				continue;
			}


			vec3f d1 = vertices[id1].p-p; d1.normalize();
			vec3f d2 = vertices[id2].p-p; d2.normalize(); 
			if(fabs(d1.dot(d2))>0.999/*С��2.85�ȼн�*/) return true; //���������{id1,p,id2}���˻�������(�н�̫С)
			
			//�·�����t.n�Ƚ�
			vec3f n;
			n.cross(d1,d2);
			n.normalize();
			deleted[k]=0;							
			if(n.dot(t.n)<0.2/*����ƫת����78.5��*/) return true;
		}
		return false;
	}

	// Update triangle connections and edge error after a edge is collapsed
	// todo:���¶���i0�ϵĶ���������
	void update_triangles(int i0,Vertex &v,const std::vector<int> &deleted,int &deleted_triangles)
	{
		vec3f p;
		loopk(0,v.tcount)
		{
			//ͨ����v������one-ring corner�õ�one-ring������
			Ref &r=refs[v.tstart+k];
			Triangle &t=triangles[r.tid]; 

			//�Ѿ�ɾ���������β�����
			if(t.deleted)continue;

			//��Ҫ��ʧ�������β��ô���
			if(deleted[k]) 
			{
				t.deleted=1; //���ɾ��
				deleted_triangles++;
				continue;
			}

			//���˵�v���ڵĶ��������滻��i0
			t.v[r.tvertex]=i0;

			//������t���Ϊdirty
			t.dirty=1;

			//���¼���������t���ߵĶ����������ŵ�
			t.err[0]=calculate_error(t.v[0],t.v[1],p);
			t.err[1]=calculate_error(t.v[1],t.v[2],p);
			t.err[2]=calculate_error(t.v[2],t.v[0],p);
			t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));

			//���仯��(������)corner׷�ӵ�refs
			refs.push_back(r);
		}
	}

	// compact triangles, compute edge error and build reference list
	void update_mesh(int iteration)
	{		
		if(iteration>0) // compact triangles
		{
			int dst=0;
			loopi(0,triangles.size())
			if(!triangles[i].deleted)
			{
				//todo:���i==dst���Լ���һ�θ�ֵ����
				//���������������,һ����ĳ��������ɾ���������
				triangles[dst++]=triangles[i];
			}
			triangles.resize(dst);
		}
		//
		// Init Quadrics by Plane & Edge Errors
		//
		// required at the beginning ( iteration == 0 )
		// recomputing during the simplification is not required,
		// but mostly improves the result for closed meshes
		//
		/*
			ͨ��ƽ���Լ����ߵĶ����������ʼ�������ͶԳƾ���

			ֻ��Ҫ�ڵ�һ�ε�������֮ǰ�ռ����������Ϣ��һЩ����������Ϣ,�ڻ��������û��Ҫ
			���½��м�������ֻ������αߵĶ��������Ϣ;�������¼������������ɻ�õ���������
			ԭʼģ�͵Ļ���Ч��;(����Ҫ�ڻ���Ч�ʺͻ���Ч��֮�����ƽ��,������򼶱��������
			���������ȿ���Ч��,��֮,ʮ�򼶱���������������ȿ���Ч��)
		*/
		if( iteration == 0 )
		{
			//�����ƽ������
			loopi(0,vertices.size()) 
			vertices[i].q=SymetricMatrix(0.0);

			//��ÿ��������ƽ���ۼӵ�������û��������
			loopi(0,triangles.size()) 
			{
				//��ȡ�����εĶ���λ����Ϣ
				Triangle &t=triangles[i]; 
				vec3f n,p[3];
				loopj(0,3) p[j]=vertices[t.v[j]].p;
				
				//��������������ƽ��ĵ�λ����
				n.cross(p[1] - p[0], p[2] - p[0]);
				n.normalize();
				t.n=n; 

				//�������εĶ������ۼ�ƽ��
				loopj(0,3) vertices[t.v[j]].q = 
					vertices[t.v[j]].q+SymetricMatrix(n.x,n.y,n.z,-n.dot(p[0]));
			}

			//����������ÿ����������ʱ�����ŵ㼰��������(��������)
			loopi(0,triangles.size())
			{
				// Calc Edge Error
				Triangle &t=triangles[i];vec3f p;
				loopj(0,3) t.err[j]=calculate_error(t.v[j],t.v[(j+1)%3],p);  //����ÿ���ߵ�����ֵ
				t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));               //���������ε�����ֵ
			}	
		}

		// Init Reference ID list	
		loopi(0,vertices.size())
		{
			vertices[i].tstart=0;
			vertices[i].tcount=0;
		}

		//ͳ��ÿ�����������ӵ�����������
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3) vertices[t.v[j]].tcount++;
		}

		//ÿ�������corner�������е���ʼ������
		int tstart=0;
		loopi(0,vertices.size())
		{
			Vertex &v=vertices[i];
			v.tstart=tstart;
			tstart+=v.tcount;
			v.tcount=0; //����v��corner��������
		}

		// Write References
		/*��¼����vertex�������corner{tid,tvertexOrder}
		�ռ������corner����(ref)��Ȼ���ն�������˳��洢��vector��;
		ÿ�������corner������¼�ڶ���v.tcount��;
		ref.tid��¼corner���������ε�������;
		ref.tvertex��¼corner�Ķ����ڸ�������tid�����[0...2];
		*/
		refs.resize(triangles.size()*3);
		loopi(0,triangles.size())
		{
			//������
			Triangle &t=triangles[i];	
			loopj(0,3)
			{
				//corner
				Vertex &v=vertices[t.v[j]];
				refs[v.tstart+v.tcount].tid=i;     //���������
				refs[v.tstart+v.tcount].tvertex=j; //�������[0..2]
				v.tcount++;
			}
		}

		// Identify boundary : vertices[].border=0,1 
		//�߽�̽��
		/*
		todo:
		1)��Ҫ���ӱ����ֻ��һ������������,����Ϊ�߽�,��Ҫ���Ӵ�ֱƽ������ֹ�����ڲ����ߵĲ���;
		2)��ĳ��������ĳ��������֮��Ķ����С��5������Ҫ���Ӵ�ֱƽ������ֹ�����������߲���(��Ҫ
		���Ǵ�ʱ������������������������Ƿ��������ʱ,�Ƕȵļ�������);
		*/
		if( iteration == 0 )
		{
			std::vector<int> vcount,vids;

			loopi(0,vertices.size())
				vertices[i].border=0;

			loopi(0,vertices.size())
			{
				//����v
				Vertex &v=vertices[i];

				/*�ռ�����v������corner����������(����v��one-ring������)������
				������������뼰����ֵ�����(���ֵĴ���)*/
				vcount.clear();  //��Ӧ�������(multiplicity)
				vids.clear();    //���㼯��(vid set)

				//����v������corner
				loopj(0,v.tcount)
				{
					//corner��Ӧ�����������
					int k=refs[v.tstart+j].tid;
					Triangle &t=triangles[k];

					//corner���������ε���������
					loopk(0,3)
					{
						//��vids�����в����Ƿ���idԪ��,��ͨ��ofs�±�ָʾ����λ��
						int ofs=0,id=t.v[k];
						while(ofs<vcount.size())
						{
							if(vids[ofs]==id)break; 
							ofs++;
						}

						//��vids��û�ҵ�id,��id׷�ӵ�vids��
						if(ofs==vcount.size())
						{    
							vcount.push_back(1);
							vids.push_back(id);
						}
						//��vids�е�ofsλ���ҵ���id,��ô������1
						else 
							vcount[ofs]++;
					}
				}

				/*ֻ��һ�������������Ķ�����Ϊborder==1;
				  �����ж�ĳ������ֻ�к���һ������border==1��ı߾�����Ҫ�������߲����ģ�
				  ��������㶼������border==1�ĵ�,����Ϊĳ������������(��������������ֻͨ��0����1��������ͨ),
				  ���ֹ���������Ҳ�Ƿ������߲���*/
				loopj(0,vcount.size()) if(vcount[j]==1)
					vertices[vids[j]].border=1;					
			}
		}
	}

	// Finally compact mesh before exiting
	void compact_mesh()
	{
		int dst=0;
		loopi(0,vertices.size())
		{
			vertices[i].tcount=0;
		}
		loopi(0,triangles.size())
		if(!triangles[i].deleted)
		{
			Triangle &t=triangles[i];
			triangles[dst++]=t;                   //�����ζ����滻
			loopj(0,3)vertices[t.v[j]].tcount=1;
		}
		//�����ζ����ڴ�����
		triangles.resize(dst);
		dst=0;
		loopi(0,vertices.size())
		if(vertices[i].tcount)
		{
			vertices[i].tstart=dst;
			vertices[dst].p=vertices[i].p;        //ֻ�滻����ļ���λ��,������Ϣ����
			dst++;
		}
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3)t.v[j]=vertices[t.v[j]].tstart;
		}
		//��������ڴ�����
		vertices.resize(dst);
	}

	// Error between vertex and Quadric
	// �����{x,y,z}��ƽ��{a,b,c,d}��ƽ������
	double vertex_error(SymetricMatrix q, double x, double y, double z)
	{
 		return   q[0]*x*x + 2*q[1]*x*y + 2*q[2]*x*z + 2*q[3]*x + q[4]*y*y
 		     + 2*q[5]*y*z + 2*q[6]*y + q[7]*z*z + 2*q[8]*z + q[9];
	}

	// Error for one edge
	// �������߶�{id_v1,id_v2}��ĳһ��p_result
	// �������ڵ�{id_v1}�͵�{id_v2}������ƽ�����̾���ƽ������С
	double calculate_error(int id_v1, int id_v2, vec3f &p_result)
	{
		// compute interpolated vertex 
		// �ռ���v1,v2�����ж���������
		SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;

		// v1,v2��ֻҪ��һ���Ǳ߽��(�ĵ�ֻ��һ������������)���{v1,v2}Ϊborder
		bool   border = vertices[id_v1].border & vertices[id_v2].border;
		double error=0;

		//����4*4����q��3*3�Ӿ��������ʽ
		double det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);

		if ( det != 0 && !border )
		{
			// q_delta is invertible(����ķ��������������Է�����,���Simplify.GL-SymetricMatrix.png֮�Ƶ�)
			p_result.x = -1/det*(q.det(1, 2, 3, 4, 5, 6, 5, 7 , 8));	// vx = A41/det(q_delta) 
			p_result.y =  1/det*(q.det(0, 2, 3, 1, 5, 6, 2, 7 , 8));	// vy = A42/det(q_delta) 
			p_result.z = -1/det*(q.det(0, 1, 3, 1, 4, 6, 2, 5,  8));	// vz = A43/det(q_delta) 
			error = vertex_error(q, p_result.x, p_result.y, p_result.z);
		}
		else
		{
			// det = 0 -> try to find best result
			// ���q�л������������(�κ�һ�Ӿ�������ʽ������ػ�������ʽΪ��)����border==true,
			// ȡ�����˵���е��о���ƽ������С��Ϊ���ŵ�
			vec3f p1=vertices[id_v1].p;
			vec3f p2=vertices[id_v2].p;
			vec3f p3=(p1+p2)/2;

			// �ֱ�������ƽ����
			double error1 = vertex_error(q, p1.x,p1.y,p1.z);
			double error2 = vertex_error(q, p2.x,p2.y,p2.z);
			double error3 = vertex_error(q, p3.x,p3.y,p3.z);

			// ȡ����ƽ������Сֵ
			error = min(error1, min(error2, error3));

			// ���ؾ���ƽ������С�ĵ�{v1,v2,�е�}
			if (error1 == error) p_result=p1;
			if (error2 == error) p_result=p2;
			if (error3 == error) p_result=p3;
		}
		return error;
	}

	// Optional : Store as OBJ
	void write_obj(char* filename)
	{
		FILE *file=fopen(filename, "w");
		if (!file)
		{
			printf("write_obj: can't write data file \"%s\".\n", filename);
			system("PAUSE");
			exit(0);
		}
		//�������������Ϣ
		loopi(0,vertices.size())
		{
			fprintf(file, "v %lf %lf %lf\n", vertices[i].p.x,vertices[i].p.y,vertices[i].p.z);
		}	
		//�������������������Ϣ
		loopi(0,triangles.size()) if(!triangles[i].deleted)
		{
			fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1);
		}
		fclose(file);
	}
};
///////////////////////////////////////////
