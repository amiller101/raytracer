// Tool for parsing .obj files into triangle meshes.
// Creates triangle mesh with smooth triangles if .obj file contains vertex normals.

#define TINYOBJLOADER_IMPLEMENTATION 
#include "external/tiny_obj_loader.h"

#include "hittable.h"
#include "triangle.h"
#include "bvh.h"

auto default_mat = make_shared<lambertian>(color(0.8, 0.8, 0.8));
bool smooth = true;

shared_ptr<hittable> load_obj_mesh(const std::string& filename, bool smooth)
{
    tinyobj::ObjReaderConfig config;
    config.triangulate = true; // ensures all faces are triangles
    config.vertex_color = false;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, config)) {
        if (!reader.Error().empty()) {
            throw std::runtime_error("TinyOBJ error: " + reader.Error());
        }
        throw std::runtime_error("Failed to load OBJ file.");
    }

    if (!reader.Warning().empty()) {
        std::cerr << "TinyOBJ warning: " << reader.Warning() << "\n";
    }

    // Array of vertices in the mesh
    const auto& attrib = reader.GetAttrib();
    
    // Array of indices triplets that index vertices in attrib for points of a triangle.
    const auto& shapes = reader.GetShapes();

    const auto& materials = reader.GetMaterials();

    std::vector<shared_ptr<material>> material_table;

    for (const auto& m : materials) {
        shared_ptr<texture> diffuse_tex;

        if (!m.diffuse_texname.empty()) {
            std::string path =  "../assets/models/" + m.diffuse_texname;
            diffuse_tex = make_shared<image_texture>(path.c_str());
        } else {
            diffuse_tex = make_shared<solid_color>(
                color(m.diffuse[0], m.diffuse[1], m.diffuse[2])
            );
        }

        material_table.push_back(
            make_shared<lambertian>(diffuse_tex)
        );
    }


    shared_ptr<hittable_list> tris = make_shared<hittable_list>();

    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f]; // usually 3

            tinyobj::index_t idx0 = shape.mesh.indices[index_offset + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[index_offset + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[index_offset + 2];

            // ----- positions -----
            auto get_v = [&](tinyobj::index_t idx) {
                return point3(
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                );
            };

            point3 v0 = get_v(idx0);
            point3 v1 = get_v(idx1);
            point3 v2 = get_v(idx2);

            //materials
            int mat_id = shape.mesh.material_ids[f];
            shared_ptr<material> mat;

            if (mat_id >= 0 && mat_id < material_table.size())
                mat = material_table[mat_id];
            else
                mat = default_mat;


            // ----- normals -----
            bool has_normals =
                idx0.normal_index >= 0 &&
                idx1.normal_index >= 0 &&
                idx2.normal_index >= 0;

            if (has_normals && smooth) {
                auto get_n = [&](tinyobj::index_t idx) {
                    return Vec3(
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    );
                };

                Vec3 n0 = get_n(idx0);
                Vec3 n1 = get_n(idx1);
                Vec3 n2 = get_n(idx2);

                tris->add(
                    make_shared<Smooth_Triangle>(v0, v1, v2, n0, n1, n2, mat)
                );

            } else {
                tris->add(
                    make_shared<Triangle>(v0, v1, v2, mat)
                );
            }

            index_offset += fv;
        }
    }

    // return hittables list
    return tris;
}
