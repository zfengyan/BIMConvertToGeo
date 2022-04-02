#include <iostream>
#include <fstream>

#include "json.hpp"
#include "Polyhedra.hpp"


// shells for writing to json 
struct JShell {
	std::vector<std::vector<unsigned long>> faces;
};


class WriteToJSON {
public:
	std::vector<Point> vertices; // vertices for writing to city json file
	std::vector<JShell> jshells; // selected shells for writing to city json file
private:
	/*
	* check if a vertex already exists in a vertices vector
	* USE coordinates to compare whether two vertices are the same
	* return: False - not exist, True - exist
	*/
	bool vertex_exist_check(std::vector<Point>& vertices, Point& vertex) {
		bool flag(false);
		for (auto& v : vertices) {
			if (
				abs(vertex.x() - v.x()) < Epsilon &&
				abs(vertex.y() - v.y()) < Epsilon &&
				abs(vertex.z() - v.z()) < Epsilon) {

				flag = true;
			}
		}
		return flag;
	}


	/*
	* if a vertex is repeated, find the index and return
	*/
	unsigned long find_vertex(std::vector<Point>& vertices, Point& vertex) {
		for (std::size_t i = 0; i != vertices.size(); ++i) {
			if (
				abs(vertex.x() - vertices[i].x()) < Epsilon &&
				abs(vertex.y() - vertices[i].y()) < Epsilon &&
				abs(vertex.z() - vertices[i].z()) < Epsilon) {

				return (unsigned long)i;
			}
		}
		std::cout << "warning: please check find_vertex function" << '\n';
		return 0;
	}

public:
	/*
	* select the se which is needed to be written to cityjson
	* add non-repeated vertices to vertices list
	* add the correct indices of each face in each shell
	*
	* selected shell explorer:
	* shell_explorers[0] - exterior
	* shell_explorers[1] - room
	* shell_explorers[4] - room
	* shell_explorers[5] - room
	* shell_explorers[6] - room
	*/
	void process_shell_explorer_indices(std::vector<Shell_explorer>& shell_explorers)
	{
		// first store all the vertices in a vector
		std::vector<Point> all_vertices; // contains repeated vertices
		for (auto const& se : shell_explorers) {
			for (auto const& v : se.vertices) {
				all_vertices.push_back(v);
			}
		}

		// next store the face indexes(accumulated from 0)	
		unsigned long index_in_all_vertices = 0;
		for (auto& se : shell_explorers) {
			for (auto& face : se.faces) {
				for (auto& index : face) {
					index = index_in_all_vertices++;
				}
			}
		}
		// now we have the all_vertices and shell_explorers to write to cityjson -----------------------------


		// clear the repeated vertices, add them to vertices(param), add the selected shells to shells(param)
		unsigned long index_in_vertices = 0;

		// exterior ----------------------------------------
		auto const& se_0 = shell_explorers[0];
		JShell jshell_0; // corresponds to se_0
		for (auto const& current_face : se_0.faces) {
			jshell_0.faces.emplace_back();
			for (auto const& current_index : current_face) {
				Point& vertex = all_vertices[current_index];
				if (!vertex_exist_check(vertices, vertex)) {
					vertices.push_back(vertex);
					jshell_0.faces.back().push_back(index_in_vertices++);
				}
				else {
					unsigned long exist_vertex_index = find_vertex(vertices, vertex);
					jshell_0.faces.back().push_back(exist_vertex_index);
				}
			}
		}
		jshells.push_back(jshell_0);

	}


	/*
	* write the vertices and selected jshells to city json
	*/
	void write_vertices_shells(std::string& fname) {
		// basic info ---------------------------------------------------------------
		nlohmann::json json;
		json["type"] = "CityJSON";
		json["version"] = "1.1";
		json["transform"] = nlohmann::json::object();
		json["transform"]["scale"] = nlohmann::json::array({ 1.0, 1.0, 1.0 });
		json["transform"]["translate"] = nlohmann::json::array({ 0.0, 0.0, 0.0 });
		json["vertices"] = nlohmann::json::array({}); // vertices

		// all vertices(including repeated vertices)-----------------------------------		
		for (auto const& v : vertices) {
			double x = CGAL::to_double(v.x()); // warning: may have precision loss
			double y = CGAL::to_double(v.y());
			double z = CGAL::to_double(v.z());
			json["vertices"].push_back({ x, y, z });
		}

		// Building info -------------------------------------------------------------
		json["CityObjects"] = nlohmann::json::object();
		json["CityObjects"]["Building_1"]["type"] = "Building";
		json["CityObjects"]["Building_1"]["attributes"] = nlohmann::json({});
		json["CityObjects"]["Building_1"]["children"] = nlohmann::json::array({ "Building_1_0" }); // { "Building_1_0", "Building_1_1" }
		json["CityObjects"]["Building_1"]["geometry"] = nlohmann::json::array({});

		// BuildingPart - exterior------------------------------------------------------
		json["CityObjects"]["Building_1_0"]["type"] = "BuildingPart";
		json["CityObjects"]["Building_1_0"]["attributes"] = nlohmann::json({});
		json["CityObjects"]["Building_1_0"]["parents"] = nlohmann::json::array({ "Building_1" });
		json["CityObjects"]["Building_1_0"]["geometry"] = nlohmann::json::array();
		json["CityObjects"]["Building_1_0"]["geometry"][0]["type"] = "Solid";
		json["CityObjects"]["Building_1_0"]["geometry"][0]["lod"] = "2.2";
		json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"] = nlohmann::json::array({}); // indices	
		auto& b_BuildingPart = json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"][0];
		auto const& jshell_exterior = jshells[0];
		for (auto const& face : jshell_exterior.faces) {
			b_BuildingPart.push_back({ face });
		}

		// write to file
		std::string json_string = json.dump(2);
		std::ofstream out_stream(OUTPUT_PATH + fname);
		out_stream << json_string;
		out_stream.close();
	}
};



void write_to_json(std::vector<Shell_explorer>& shell_explorers) {
	// basic info ---------------------------------------------------------------
	nlohmann::json json;
	json["type"] = "CityJSON";
	json["version"] = "1.1";
	json["transform"] = nlohmann::json::object();
	json["transform"]["scale"] = nlohmann::json::array({ 1.0, 1.0, 1.0 });
	json["transform"]["translate"] = nlohmann::json::array({ 0.0, 0.0, 0.0 });
	json["vertices"] = nlohmann::json::array({}); // vertices

	// all vertices(including repeated vertices)-----------------------------------
	for (auto const& se : shell_explorers) {
		for (auto& v : se.vertices) {
			double x = CGAL::to_double(v.x()); // warning: may have precision loss
			double y = CGAL::to_double(v.y());
			double z = CGAL::to_double(v.z());
			json["vertices"].push_back({ x, y, z });
		} // vertices may be repeated
	}

	// Building info -------------------------------------------------------------
	json["CityObjects"] = nlohmann::json::object();
	json["CityObjects"]["Building_1"]["type"] = "Building";
	json["CityObjects"]["Building_1"]["attributes"] = nlohmann::json({});
	json["CityObjects"]["Building_1"]["children"] = nlohmann::json::array({ "Building_1_0", "Building_1_1" });
	json["CityObjects"]["Building_1"]["geometry"] = nlohmann::json::array({});

	// BuildingPart - exterior------------------------------------------------------
	json["CityObjects"]["Building_1_0"]["type"] = "BuildingPart";
	json["CityObjects"]["Building_1_0"]["attributes"] = nlohmann::json({});
	json["CityObjects"]["Building_1_0"]["parents"] = nlohmann::json::array({ "Building_1" });
	json["CityObjects"]["Building_1_0"]["geometry"] = nlohmann::json::array();
	json["CityObjects"]["Building_1_0"]["geometry"][0]["type"] = "Solid";
	json["CityObjects"]["Building_1_0"]["geometry"][0]["lod"] = "2.2";
	json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"] = nlohmann::json::array({}); // indices	
	auto& b_BuildingPart = json["CityObjects"]["Building_1_0"]["geometry"][0]["boundaries"][0];
	auto const& se_exterior = shell_explorers[0];
	for (auto& face : se_exterior.faces) {
		std::vector<unsigned long> one_face;
		for (auto& index : face) {
			one_face.push_back(index);
		}
		b_BuildingPart.push_back({ one_face });
		one_face.clear();
	}

	// BuildingRoom--------------------------------------------------------------------
	json["CityObjects"]["Building_1_1"]["type"] = "BuildingRoom";
	json["CityObjects"]["Building_1_1"]["attributes"] = nlohmann::json({});
	json["CityObjects"]["Building_1_1"]["parents"] = nlohmann::json::array({ "Building_1" });
	json["CityObjects"]["Building_1_1"]["geometry"] = nlohmann::json::array();
	json["CityObjects"]["Building_1_1"]["geometry"][0]["type"] = "Solid";
	json["CityObjects"]["Building_1_1"]["geometry"][0]["lod"] = "2.2";
	json["CityObjects"]["Building_1_1"]["geometry"][0]["boundaries"] = nlohmann::json::array({}); // indices																								  // faces for BuildingRoom2
	auto const& se_room2 = shell_explorers[9];
	auto& b_BuildingRoom = json["CityObjects"]["Building_1_1"]["geometry"][0]["boundaries"][0];
	for (auto& face : se_room2.faces) {
		std::vector<unsigned long> one_face;
		for (auto& index : face) {
			one_face.push_back(index);
		}
		b_BuildingRoom.push_back({ one_face });
		one_face.clear();
	}

	std::string json_string = json.dump(2);
	std::string outputname = "/mybuilding.json";
	std::ofstream out_stream(OUTPUT_PATH + outputname);
	out_stream << json_string;
	out_stream.close();
}



int main()
{
	//std::cout << "Hello CMake." << '\n';
	std::cout << "-- activated data folder: " << DATA_PATH << '\n';

	//OBJFile f; // organize vertcies, faces, shells and objects
	//
	//std::cout << '\n';
	//std::string fname = "/KIT.obj";
	//LoadOBJ::load_obj(fname, f);

	//std::cout << '\n';
	//std::string repeated_info_name = "/KIT.repeated.vertices.txt";
	//LoadOBJ::repeated_vertices_info(repeated_info_name, f);

	//std::cout << '\n';
	//std::string repeated_faces_name = "/KIT.repeated.faces.txt";
	//LoadOBJ::repeated_faces_info(repeated_faces_name, f);

	//std::cout << '\n';
	//std::string new_vertices_name = "/KIT.new.vertices.txt";
	//LoadOBJ::process_repeated_vertices(new_vertices_name, f);

	//std::cout << '\n';
	//std::string new_faces_name = "/KIT.new.faces.txt";
	//LoadOBJ::process_repeated_faces(new_faces_name, f);

	//std::cout << '\n';
	//std::string output_obj_name = "/KIT.output.obj";
	//LoadOBJ::output_obj(output_obj_name, f);

	///* 
	//* prepare the verticesand face - indices for each shell
	//* shell.poly_vertices -- store the vertices of this shell
	//* shell.faces -> face.v_poly_indices -- store the indices(0-based) point to the shell.poly_vertices 
	//*/
	//PreparePolyhedron::prepare_poly_vertices_face_indices(f); // uncomment this to output each shell
	//PreparePolyhedron::output_each_shell(f); // uncomment this to output each shell

	//std::cout << '\n';
	
	Nef nef;

	// build Nef_polyhedra according to different shells, add the nef polyhedra to nef list
	Build_Nef_Polyhedron::build_nef_polyhedra(nef);

	// build big Nef
	BigNef::test_big(nef);

	
	// extract geometries ------------------------------------------------------------
	std::vector<Shell_explorer> shell_explorers;
	
	int volume_count = 0;
	int shell_count = 0;
	Nef_polyhedron::Volume_const_iterator current_volume;
	CGAL_forall_volumes(current_volume, nef.big_nef) {
		std::cout << "volume: " << volume_count++ << " ";
		std::cout << "volume mark: " << current_volume->mark() << '\n';
		Nef_polyhedron::Shell_entry_const_iterator current_shell;
		CGAL_forall_shells_of(current_shell, current_volume) {
			Shell_explorer se;
			Nef_polyhedron::SFace_const_handle sface_in_shell(current_shell);
			nef.big_nef.visit_shell_objects(sface_in_shell, se);

			//add the se to shell_explorers
			shell_explorers.push_back(se);
		}
	}

	//process the indices and write to json file----------------------------------------
	std::string filename = "/mybuilding_e.json";
	WriteToJSON w;
	w.process_shell_explorer_indices(shell_explorers);
	w.write_vertices_shells(filename);

	std::cout << "after extracting geometries: " << '\n';
	std::cout << "shell explorers size: " << shell_explorers.size() << '\n';
	std::cout << "-------------------------------" << '\n';
	for (auto& shell : shell_explorers) {
		std::cout << "vertices size of this shell: " << shell.vertices.size() << '\n';
		std::cout << "faces size of this shell: " << shell.faces.size() << '\n';
		std::cout << '\n';
	}

	//write_to_json(shell_explorers);

	return 0;
}
