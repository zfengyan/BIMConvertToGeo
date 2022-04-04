#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

const double Epsilon = 1e-8;

// 3d vector
struct Vector3d {
	double x, y, z;

	Vector3d():
		x(0),y(0),z(0){}

	Vector3d(double vx, double vy, double vz):
		x(vx),y(vy),z(vz){}

	virtual ~Vector3d() = default;
};

// Vertex
struct Vertex : public Vector3d {
	unsigned long vid; // vertex id in vector of points
	std::string shell_id; // potential use, belongs to which shell
	std::string object_id; // potential use, belongs to which object

	unsigned long newid; // vertex index in the new vertices vector -- after repeated process

	Vertex():
		Vector3d(),
		vid(0),
		shell_id("null"),
		object_id("null"),
		newid(0)
	{}

	Vertex(double vx, double vy, double vz):
		Vector3d(vx, vy, vz),
		vid(0),
		shell_id("null"),
		object_id("null"),
		newid(0)
	{}

	Vertex(double vx, double vy, double vz, unsigned long id) :
		Vector3d(vx, vy, vz),
		vid(id),
		shell_id("null"),
		object_id("null"),
		newid(0)
	{}

	// print coordinates
	void print() const {
		std::cout << "index: " << vid << " - - "
			<< "(" << x << ", " << y << ", " << z << ")" << '\n';
	}

};


struct Face {
	std::vector<unsigned long> v_indices; // indices in a vector of points(origin from obj file)
	Vector3d obj_normal; // normal vector of a face, from obj file
	// Vector3d cal_normal; // potentially calculated normal

	std::vector<unsigned long> v_new_indices; // for repeated vertices process
	bool contain_repeated_flag; // if contains repeated vertices, set it to true

	std::vector<unsigned long> v_poly_indices; // point to each shell's vertices list, for creating polyhedron

	std::string shell_id;
	std::string obj_id;

	Face():
		shell_id("null"),
		obj_id("null"),
		contain_repeated_flag(false)
	{}
};


struct Shell {
	std::string id; // id of shell -- should be 1, 2, ... n, stands for: the i-th shell in one object
	std::string obj_id; // belongs to which object
	std::vector<Face> faces;

	std::vector<Vertex> poly_vertices; // store vertices for each shell for creating polyhedron of this shell

	Shell():
		id("null"),
		obj_id("null")
	{}
};


struct Object {
	std::string id;
	std::vector<Shell> shells;

	Object():
		id("null"){}
};


struct OBJFile {
	std::vector<Vertex> vertices; // store all vertices in obj file
	std::vector<Face> faces;
	std::vector<Shell> shells;
	std::vector<Object> objects;

	// -- for repeated process
	std::vector<std::vector<Vertex>> repeated_vertices; // store repeated vertices
	std::vector<Vertex> new_vertices; // store no repeated vertices

	//std::map<unsigned long, Vertex> vertices_dict; // potential use
	//std::vector<Face> repeated_faces; // potential use -- store repeated faces
};

// load obj files and process repeated vertices
class LoadOBJ {
private:
	/*
	* spilt a string according to a certain delimiter
	* ie input: "a//b//c", delimiter = "//"
	* return: a vector contains "a", "b", "c"
	*/
	static std::vector<std::string> string_split(std::string& s, std::string& delimiter) {
		std::vector<std::string> result;

		std::size_t pos = 0;
		std::string token;
		while ((pos = s.find(delimiter)) != std::string::npos) {
			token = s.substr(0, pos);
			result.emplace_back(token);
			s.erase(0, pos + delimiter.length());
		}
		result.emplace_back(s);

		return result;
	}


	/*
	* check if a vertex already exists in a vertices vector
	* USE coordinates not indices to compare whether two vertices are the same
	* return: False - not exist, True - exist
	*/
	static bool vertex_exist_check(std::vector<Vertex>& vertices, Vertex& vertex) {
		bool flag(false);
		for (auto& v : vertices) {
			if (
				abs(vertex.x - v.x) < Epsilon &&
				abs(vertex.y - v.y) < Epsilon &&
				abs(vertex.z - v.z) < Epsilon) {

				flag = true;
			}
		}
		return flag;
	}


	/*
	* check if a vertex already exists in repeated vertices vector -- based on vid, NOT coordinates
	*/
	static bool repeated_vertex_exist_check(std::vector<std::vector<Vertex>>& repeated_vertices, Vertex& vertex) {
		bool flag(false);
		for (auto& one_set : repeated_vertices) {
			for (auto& v : one_set) {
				if (vertex.vid == v.vid){
					flag = true;
				}
			}
		}
		return flag;
	}

public:

	/*
	* load vertices, faces, shells and objects
    * coordinates of vertices may REPEATE in vertices vector
	*/
	static void load_obj(std::string& fname, OBJFile& f) {
		std::string path = INPUT_PATH;
		std::string filename = path + fname;
		std::cout << "-- loading obj file: " << filename << '\n';

        std::string line;
        std::ifstream file(filename);
        if (!file.is_open()) { std::cerr << "file open failed! " << '\n'; }

		unsigned long vertex_index = 1; // track vertex index in obj file, starts from 1	   
		std::vector<double> coordinates; // store xyz coordinates of each vertex line
		std::vector<unsigned long> face_v_indices; // store face-vertex indices in each face line
		
		// for adding faces - shells
		std::vector<Face> tmp_faces; // store faces of each shell
		std::vector<std::vector<Face>> all_faces; // store all faces, grouped by shells 				

		// process each line in the obj file
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue; // skip empty lines:
            }

			// use markers below to mark the type of each line
			bool v_flag(false); // entitled "v"
			bool vn_flag(false); // entitled "vn"
			bool f_flag(false); // entitled "f"
			bool s_flag(false); // entitled "s"
			bool g_flag(false); // entitled "g"
			
			// help to process elements in each line
            std::stringstream ss(line);
            std::string field; // element in each line
            std::string::size_type sz; // NOT std::size_t, size of std::string

			// for each element(field, type: string) in one line
            while (std::getline(ss, field, ' ')) { 

				// get the type of current line
				if (field == "g") {
					g_flag = true;
					continue; // jump to next field in this line
				}
				else if (field == "s") { 
                    s_flag = true; 
                    continue; 
                }
				else if (field == "f") {
					f_flag = true;
					continue;
				}
				else if (field == "v") {
					v_flag = true;
					continue;
				}

				// process the current line
				if (g_flag) {
					Object obj;
					obj.id = field;
					f.objects.emplace_back(obj);
				}
				else if (s_flag) {
					
					// add shell to f.shells
					Shell s;
					s.id = field;
					f.shells.emplace_back(s);

					// add faces to all_faces and reset the tmp_faces vector
					all_faces.emplace_back(tmp_faces);
					tmp_faces.clear();
							
				}
                else if (v_flag) {
                    // process xyz coordinates
                    coordinates.emplace_back(std::stod(field, &sz));
                }
				else if (f_flag) {

					// each field(skipped "f" : 4//4, 5//5, 6//6)
					std::string delimiter = "//";
					std::vector<std::string> splitted = string_split(field, delimiter);
					//std::cout << splitted[0] << " " << splitted[1] << " ";

					face_v_indices.emplace_back((unsigned long)std::stoi(splitted[0], &sz));
				}
            
            } // end while: process each element in one line

            // process each vertex (if it's a vertex line)
            if (!coordinates.empty() && coordinates.size() == 3) {
                f.vertices.emplace_back(
					Vertex(
						coordinates[0], // x
						coordinates[1], // y
						coordinates[2], // z
						vertex_index) // vertex index in obj file
				);
				++vertex_index;
            }
			coordinates.clear();

			// constrcut face and add it into file.faces (if it's a face line)
			if (!face_v_indices.empty()) {
				Face face;
				face.v_indices = face_v_indices;
				f.faces.emplace_back(face);

				// store the faces and for adding them to shells
				// once added to a shell, tmp_faces should be cleared
				tmp_faces.emplace_back(face);
			}
			face_v_indices.clear();

        } // end while: each line in the file


		// --------------------------------- end while(each line is processed) --------------------------------
		
		// construct shell - faces ************************************************
		// add the last group of faces in all_faces
		all_faces.emplace_back(tmp_faces);

		// all_faces and shells:
		//std::cout << "shell size: " << f.shells.size() << '\n';
		//std::cout << "all faces size: " << all_faces.size() << '\n';

		// construct shell - faces
		// the first vector in all_faces is null
		// ie : shell_0.faces, shell_1.faces, shell_2.faces, corresponds to all_faces[1], all_faces[2], all_faces[3]
		std::size_t size_all_faces = all_faces.size();
		std::size_t size_shells = f.shells.size();
		if (size_all_faces - size_shells == 1)
		{
			for (std::size_t i = 0; i != size_shells; ++i)
			{
				f.shells[i].faces = all_faces[i + 1];
			}
		}
		else {
			std::cout << "loading obj file error, please check " << '\n';
		}

		// construct object - shells ************************************************
		// ie, obj: 0, 1, 2, 3, 4 -- each obj has its own unique id
		// shell id: 1, 1, 1, 2, 1, 1 -- each obj can have more than one shell, obj 2 has two shells		
		// std::cout << "object size: " << f.objects.size() << '\n';
		
		std::size_t oindex = 0; // obj index in f.objects
		std::size_t sindex = 0; // shell index in f.shells
		
		while (oindex != f.objects.size()) 
		{
			Object& obj = f.objects[oindex]; // get the current object in f.objects

			while (sindex != f.shells.size())
			{
				Shell& shell = f.shells[sindex]; // current shell in f.shells
				
				if ((shell.id.compare("1") == 0 && obj.shells.empty()) ||
					shell.id.compare("1") != 0){
					obj.shells.emplace_back(shell);
					++sindex;
				}
				else if (shell.id.compare("1") == 0 && (!obj.shells.empty())) {
					++oindex; // go to next obj
					break;
				}
			
			}

			// condition to quit the loop
			if (oindex + 1 == f.objects.size() && sindex >= f.shells.size())break;
		}

		std::cout << "loading obj file done " << '\n';
		
	}


	/*
	* repeated vertices information
	* store the repeated vertices in f.repeated_vertices
	* ie v1 and v2 are repeated -- store v1, v2 in a map v1 : v2
	*/
	static void repeated_vertices_info(std::string& fname, OBJFile& f) {
		
		std::cout << "-- repeated vertcies check: " << '\n';
		std::cout<< "Epsilon threshold: " << Epsilon << '\n';
		
		std::string path = INTER_PATH;
		std::string filename = path + fname;

		std::ofstream myfile;
		myfile.open(filename);

		myfile << "-- repeated vertcies check: " << '\n';
		myfile << "Epsilon threshold: " << Epsilon << '\n';

		unsigned long repeated_count = 0;
		std::vector<Vertex>::iterator it = f.vertices.begin();
		std::vector<Vertex> one_set_repeated; // store a set of repeated vertices with same coordinates	

		while (it != f.vertices.end()) {
			
			Vertex& base_v = *it; // each base vertex
			std::vector<Vertex>::iterator nxt = std::next(it); // compare vertex starts from the next vertex of current base vertex

			while (nxt != f.vertices.end()) {
				Vertex& compare_v = *nxt; 
				if (abs(compare_v.x - base_v.x) < Epsilon &&
					abs(compare_v.y - base_v.y) < Epsilon &&
					abs(compare_v.z - base_v.z) < Epsilon) {
					
					if (one_set_repeated.empty() && (!repeated_vertex_exist_check(f.repeated_vertices, base_v)))
					{
						one_set_repeated.emplace_back(base_v);
					}
					if (!one_set_repeated.empty()) { // if base_v exist
						one_set_repeated.emplace_back(compare_v);
						repeated_count += 1;
					}
				}
				++nxt;
			}

			if (!one_set_repeated.empty())f.repeated_vertices.emplace_back(one_set_repeated);
			one_set_repeated.clear();
			++it;
		}

		// write repeated vertices info
		for (auto& one_set : f.repeated_vertices) {
			myfile << "repeated: " << '\n';
			for (auto& v : one_set) {
				myfile << v.vid << " " << "(" << v.x << ", " << v.y << ", " << v.z << ")" << '\n';
			}
		}

		myfile << "repeated vertices count: " << repeated_count << '\n';
		myfile.close();

		std::cout << "repeated vertices count: " << repeated_count << '\n';
		std::cout << "repeated vertices info stored in: " << filename << '\n';
	}	


	/*
	* repeated faces -- faces containing repeated vertices
	*/
	static void repeated_faces_info(std::string& fname, OBJFile& f) {
		std::cout << "-- faces repeated check: " << '\n';
		
		// process faces containing repeated vertices -- add indices to face.v_new and set the face.contain_repeated_flag
		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				for (auto& face : shell.faces)
				{
					for (auto& indice : face.v_indices)
					{
						bool repeated_indice_flag(false); // if current indice is in f.repeated_vertices					
						
						for (auto& one_set : f.repeated_vertices) {
							for (auto& v : one_set) {
								if (indice == v.vid) {
									face.v_new_indices.emplace_back(one_set[0].vid); // use the first vertex in each repeated vertex set as the new indice
									face.contain_repeated_flag = true;
									repeated_indice_flag = true;
								}
							}
						}
						
						// if not repeated vertex, directly add it into face.v_new
						if (!repeated_indice_flag)face.v_new_indices.emplace_back(indice);
					}
						
				}
			}
		}

		// write faces containing repeated vertices
		std::string path = INTER_PATH;
		std::string filename = path + fname;
		std::ofstream myfile;
		myfile.open(filename);
		myfile << "faces repeated check info: " << '\n';
		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				for (auto& face : shell.faces)
				{
					if (face.contain_repeated_flag) { // if it's a face containing repeated vertices
						myfile << "original: " << "f" << " ";
						for (auto& indice : face.v_indices)
						{
							myfile << indice << " ";
						}
						myfile << '\n';

						myfile << "new: " << "f" << " ";
						for (auto& indice : face.v_new_indices)
						{
							myfile << indice << " ";
						}
						myfile << '\n';
					}					
				}
			}
		}
	
		std::cout << "faces repeated check info stored in: " << filename << '\n';
		myfile.close();

	}


	/*
	* process repeated vertices
	* result: f.new_vertices -- containing unique vertices
	*/
	static void process_repeated_vertices(std::string& fname, OBJFile& f) {
		std::cout << "-- process repeated vertices: " << '\n';
		
		// vertex vid(1-based) - corresponding index(0-based) in f.vertices = 1
		// step 1: assign newid of vertex in new vertices
		unsigned long new_indice = 1; // new indice of vertices
		for (auto& v : f.vertices) { // modify the newid attribute in f.vertices
			bool repeated_v_falg(false);

			// if the vertex is repeated
			for (auto& one_set : f.repeated_vertices) {
				for (auto& rv : one_set) {
					if (v.vid == rv.vid) { // if the vertex is in repeated vertices list
						repeated_v_falg = true; // the vertex is repeated
						Vertex& replace_v = one_set[0]; // use the first vertex in one set as replaced vertex
						bool exist_v = vertex_exist_check(f.new_vertices, replace_v);
						if (!exist_v) { // if not already exists in f.new_vertices
							v.newid = new_indice;
							f.new_vertices.emplace_back(v);
							++new_indice;
						}
					}
				}
			}
			
			// if the vertex is unrepeated vertex
			if (!repeated_v_falg) { 
				v.newid = new_indice;
				f.new_vertices.emplace_back(v);
				++new_indice;
			}
			
		}

		// write new vertices
		std::string path = INTER_PATH;
		std::string filename = path + fname;
		std::ofstream myfile;
		myfile.open(filename);

		myfile << "vertices size(including repeated): " << f.vertices.size() << '\n';
		myfile << "new vertices size(not repeated): " << f.new_vertices.size() << '\n';
		myfile << "number of difference: " << f.vertices.size() - f.new_vertices.size() << '\n';
		for (auto& v : f.new_vertices) {
			//myfile << "vid: " << v.vid << " " << "(" << v.x << ", " << v.y << ", " << v.z << ")" << '\n';
			myfile << "new id: " << v.newid << " " << "(" << v.x << ", " << v.y << ", " << v.z << ")" << '\n';
		}
		myfile.close();

		std::cout << "new vertices stored in: " << filename << '\n';

		// -- re-check --------------------------------------------------------------
		unsigned long count = 0;
		for (auto it = f.new_vertices.begin(); it != f.new_vertices.end(); ++it) {
			Vertex& base_v = *it;
			auto nxt = std::next(it);
			for (; nxt != f.new_vertices.end(); ++nxt) {
				Vertex& compare_v = *nxt;
				if (abs(compare_v.x - base_v.x) < Epsilon &&
					abs(compare_v.y - base_v.y) < Epsilon &&
					abs(compare_v.z - base_v.z) < Epsilon) {
					count += 1;
				}
			}
		}
		std::cout << "recheck for the repeated vertices: " << '\n';
		std::cout << "repeated count: " << count << "\n";

	}


	/*
	* process faces containing repeated vertices
	*/
	static void process_repeated_faces(std::string& fname, OBJFile& f) {
		std::cout << "-- process faces containing repeated vertices: " << '\n';

		// access faces from the objects, thus the whole structure can be modified -- not directly use f.faces
		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				for (auto& face : shell.faces)
				{
					for (auto& indice : face.v_new_indices) {
						unsigned long index = indice - 1;
						if (index >= 0 && index != f.vertices.size()) {
							indice = f.vertices[index].newid; // point to vertices in f.new_vertices using newid
						}
						else {
							std::cout << "warning: please check process_repeated_faces function" << '\n';
							std::cout << "wrong index: " << index << '\n';
						}						
					}
				}
			}
		}

		// write new faces 
		std::string path = INTER_PATH;
		std::string filename = path + fname;
		std::ofstream myfile;
		myfile.open(filename);

		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				for (auto& face : shell.faces)
				{
					myfile << "f ";
					for (auto& indice : face.v_new_indices) {
						myfile << indice << " ";
					}
					myfile << '\n';
				}
			}
		}

		myfile.close();
		std::cout << "faces(new indices) stored in: " << filename << '\n';

	}


	/*
	* output stored elemetns to verify if it's correct
	*/
	static void output_obj(std::string& fname, OBJFile& f) {
		std::string path = INTER_PATH;
		std::string filename = path + fname;
		std::cout << "output obj file: " << '\n';

		std::ofstream myfile;
		myfile.open(filename);

		for (auto& v : f.new_vertices)
			myfile << "v" << " " << v.x << " " << v.y << " " << v.z << '\n';

		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				for (auto& face : shell.faces)
				{
					myfile << "f" << " ";
					for (auto& indice : face.v_new_indices)
						myfile << indice << " ";
					myfile << '\n';
				}
			}
		}

		myfile.close();

		std::cout << "obj file stored in: " << filename << '\n';

	}


};

// prepare vertices and faces for creating polyhedron
// use shell.vertices and face.v_poly_indices
class PreparePolyhedron {
private:
	/*
	* check if a vertex already exists in a vertices vector
	* USE coordinates not indices to compare whether two vertices are the same
	* return: False - not exist, True - exist
	*/
	static bool vertex_exist_check(std::vector<Vertex>& vertices, Vertex& vertex) {
		bool flag(false);
		for (auto& v : vertices) {
			if (
				abs(vertex.x - v.x) < Epsilon &&
				abs(vertex.y - v.y) < Epsilon &&
				abs(vertex.z - v.z) < Epsilon) {

				flag = true;
			}
		}
		return flag;
	}


	/*
    * if a vertex is repeated, find the index and return
    */
	static unsigned long find_vertex(std::vector<Vertex>& vertices, Vertex& vertex) {
		for (std::size_t i = 0; i != vertices.size(); ++i) {
			if (
				abs(vertex.x - vertices[i].x) < Epsilon &&
				abs(vertex.y - vertices[i].y) < Epsilon &&
				abs(vertex.z - vertices[i].z) < Epsilon) {

				return (unsigned long)i;
			}
		}
		std::cout << "warning: please check find_vertex function" << '\n';
		return 0;
	}
public:

	/*
	* For each shell, get the vertices of this shell and face's indices of this shell's vertices list
	* For each shell, store the UNREPEATED vertices in shell.vertices
	* and store the indices(0-based) pointing to shell.vertices for each face of this shell
	* shell.poly_vertices: (un-repeated vertices in one shell)
	* shell.faces -> face.v_poly_indices: point to shell.poly_vertices -- 0 based NOT 1 based
	*/
	static void prepare_poly_vertices_face_indices(OBJFile& f) {
		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				unsigned long poly_indice = 0; // face.v_poly_indices
				
				for (auto& face : shell.faces)
				{
					for (auto& indice : face.v_new_indices) {
						unsigned long index = indice - 1;
						if (index >= 0 && index != f.new_vertices.size()) {

							Vertex& v = f.new_vertices[index]; // NB: use f.new_vertices
							if (!vertex_exist_check(shell.poly_vertices, v)) {
								shell.poly_vertices.emplace_back(v);
								face.v_poly_indices.emplace_back(poly_indice);
								poly_indice += 1;
							}
							else {
								unsigned long r_pos = find_vertex(shell.poly_vertices, v);
								face.v_poly_indices.emplace_back(r_pos);
							}

						}
						else {
							std::cout << "warning : index, please check prepare_poly_vertices_face_indices" << '\n';
							std::cout << "index is: " << index << '\n';
						}
					}
				}
			}
		}

	}


	/*
	* test: output each shell as one .obj file
	*/
	static void output_each_shell(OBJFile& f) {
		int shell_id = 0;
		for (auto& obj : f.objects)
		{
			for (auto& shell : obj.shells)
			{
				shell_id += 1;
				std::string path = INTER_PATH;
				std::string prefix = "/";
				std::string suffix = ".obj";
				std::string fname = std::to_string(shell_id);
				std::string filename = path + prefix + fname + suffix;
				std::cout << "-- output obj file: " << filename << '\n';

				std::ofstream myfile;
				myfile.open(filename);
				for (auto& v : shell.poly_vertices) {
					myfile << "v" << " " << v.x << " " << v.y << " " << v.z << '\n';
				}
				
				for (auto& face : shell.faces)
				{
					myfile << "f" << " ";
					for (auto& indice : face.v_poly_indices)
						myfile << indice + 1 << " ";
					myfile << '\n';
				}
				myfile.close();
			}
		}
	}
};