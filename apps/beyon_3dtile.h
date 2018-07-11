#ifndef _BEYON3DTILE_H_
#define _BEYON3DTILE_H_
#include "../yocto/ext/json.hpp"
#include "../yocto/yocto_gl.h"
using json = nlohmann::json;


namespace ygl {

//double degree2rad(double val) { return val * pi / 180.0; };

// double lati_to_meter(double diff) { return diff / 0.000000157891; };
// 
// double longti_to_meter(double diff, double lati) {
//     return diff / 0.000000156785 * std::cos(lati);
// };
// 
// double meter_to_lati(double m) { return m * 0.000000157891; };
// 
// double meter_to_longti(double m, double lati) {
//     return m * 0.000000156785 / std::cos(lati);
// };
// 
// std::vector<double> transfrom_xyz(
//     double radian_x, double radian_y, double height_min) {
//     double ellipsod_a = 40680631590769;
//     double ellipsod_b = 40680631590769;
//     double ellipsod_c = 40408299984661.4;
// 
//     const double pi = std::acos(-1);
//     double xn = std::cos(radian_x) * std::cos(radian_y);
//     double yn = std::sin(radian_x) * std::cos(radian_y);
//     double zn = std::sin(radian_y);
// 
//     double x0 = ellipsod_a * xn;
//     double y0 = ellipsod_b * yn;
//     double z0 = ellipsod_c * zn;
//     double gamma = std::sqrt(xn * x0 + yn * y0 + zn * z0);
//     double px = x0 / gamma;
//     double py = y0 / gamma;
//     double pz = z0 / gamma;
// 
//     double dx = xn * height_min;
//     double dy = yn * height_min;
//     double dz = zn * height_min;
// 
//     std::vector<double> east_mat = {-y0, x0, 0};
//     std::vector<double> north_mat = {(y0 * east_mat[2] - east_mat[1] * z0),
//         (z0 * east_mat[0] - east_mat[2] * x0),
//         (x0 * east_mat[1] - east_mat[0] * y0)};
//     double east_normal =
//         std::sqrt(east_mat[0] * east_mat[0] + east_mat[1] * east_mat[1] +
//                   east_mat[2] * east_mat[2]);
//     double north_normal =
//         std::sqrt(north_mat[0] * north_mat[0] + north_mat[1] * north_mat[1] +
//                   north_mat[2] * north_mat[2]);
// 
//     std::vector<double> matrix = {east_mat[0] / east_normal,
//         east_mat[1] / east_normal, east_mat[2] / east_normal, 0,
//         north_mat[0] / north_normal, north_mat[1] / north_normal,
//         north_mat[2] / north_normal, 0, xn, yn, zn, 0, px + dx, py + dy,
//         pz + dz, 1};
//     return matrix;
// };

// extern "C" void transform_c(
//     double center_x, double center_y, double height_min, double* ptr) {
//     double radian_x = degree2rad(center_x);
//     double radian_y = degree2rad(center_y);
//     std::vector<double> v = transfrom_xyz(radian_x, radian_y, height_min);
//     std::memcpy(ptr, v.data(), v.size() * 8);
// };

};


namespace ygl {
enum TileType { B3DM, I3DM };

/*
"b3dm".This can be used to identify the arraybuffer
    as a Batched 3D Model tile.
        
byteLen- The length of the entire tile,
    including the header,
    in bytes.

featureJsonLen--The length of the Feature Table JSON section in
        bytes.Zero indicates there is no Feature Table.

featureBinLen-The length of the Feature Table binary section in
        bytes.If featureTableJSONByteLength is zero,
    this will also be zero.

batchJsonLen--The length of the Batch Table JSON section in
        bytes.Zero indicates there is no Batch Table.

batchBinLen-- The length of the Batch Table binary section in
        bytes.If batchTableJSONByteLength is zero,
    this will also be zero.
*/
struct B3dmHeader {
    std::string magic = "b3dm";
    uint version = 1;
    uint byteLen = 0;
    uint featureJsonLen = 0;
    uint featureBinLen = 0;
    uint batchJsonLen = 0;
    uint batchBinLen = 0;
    bool save(FILE* f) {
        if (byteLen == 0) return false;
        fwrite(magic.c_str(), 1, 4, f);
        fwrite(&version, 1, 4, f);
        fwrite(&byteLen, 1, 4, f);
        fwrite(&featureJsonLen, 1, 4, f);
        fwrite(&featureBinLen, 1, 4, f);
        fwrite(&batchJsonLen, 1, 4, f);
        fwrite(&batchBinLen, 1, 4, f);
        return true;
    };
};
struct TileTable {
    json js = {};
    std::vector<unsigned char> bin;
    int align(std::string& js_str) {
        js_str = js.dump(2);
        while (js_str.length() % 4) js_str += " ";
        return js_str.size();
    }
    bool save(FILE* f) {
        if (js.empty()) return false;
        std::string js_str = "";
        int len = align(js_str);
        fwrite(js_str.c_str(), 1, len, f);
        fwrite(bin.data(), 1, bin.size(), f);
        return true;
    };
};
struct FeatureTable : TileTable {
    // batchLen = 0;   //required
};
/*The Batch Table contains per-model application-specific metadata,
indexable by batchId, that can be used for declarative styling and
application-specific use cases such as populating a UI or issuing a REST API
request. In the binary glTF section, each vertex has an numeric batchId
attribute in the integer range [0, number of models in the batch - 1]. The
batchId indicates the model to which the vertex belongs. This allows models
to be batched together and still be identifiable.*/
struct BatchTable : TileTable {};
struct B3dModel {
    B3dModel(const glTF* gl, FeatureTable& ft, BatchTable& bt);
    ~B3dModel();
    B3dmHeader header;
    FeatureTable featureTable;
    BatchTable batchTable;
    const glTF* gltf = nullptr;
    std::vector<unsigned char> glb;
    void save(FILE* f) {
        if (header.save(f)) {
            featureTable.save(f);
            batchTable.save(f);
            fwrite(glb.data(), 1, glb.size(), f);
        }
    };
};
using ygl::mat4f;
enum BoundingStyle { REGION, BOX, SPHERE };
enum RefineStyle { ADD, REPLACE };
/*
region : [ west, south, east, north, minHeight, maxHeight ]
box : [
        offsetX, -offsetY, height / 2 + minHeight,  // center
        tileWidth / 2, 0, 0,                        // width
        0, tileHeight / 2, 0,                       // depth
        0, 0, height / 2                            // height
    ]
sphere :
    [
        offsetX, -offsetY, height / 2 + minHeight,
        Math.sqrt(tileWidth* tileWidth / 4 + tileHeight * tileHeight / 4 +
                  height * height / 4)
    ]
        */
struct BoundingVolume {
    BoundingStyle style = BOX;
    std::vector<float> boundArray;
    json& convert_js(json& js) {
        switch (style) {
            case REGION:
                js["region"] = {
                    boundArray[0],
                    boundArray[1],
                    boundArray[2],
                    boundArray[3],
                    boundArray[4],
                    boundArray[5],
                };
                break;
            case BOX:
                js["box"] = {boundArray[0], boundArray[1], boundArray[2],
                    boundArray[3], boundArray[4], boundArray[5], boundArray[6],
                    boundArray[7], boundArray[8], boundArray[9], boundArray[10],
                    boundArray[11]};
                break;
            case SPHERE:
                js["sphere"] = {
                    boundArray[0], boundArray[1], boundArray[2], boundArray[3]};
                break;
        }
        return js;
    };
};
struct TileContent {
    std::string uri = "";
};
struct TileNode {
    BoundingVolume m_boundingVol;
    std::vector<double> m_transform = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    //     mat4f m_transform = {
    //         {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    float m_geometricError=200;
    RefineStyle m_refineStyle = ADD;
    TileContent m_content;
    B3dModel* m_b3dm = nullptr;
    std::vector<TileNode*> m_childNode;
    json& convert_js(json& js) {
        json bound_js;
        js["boundingVolume"] = m_boundingVol.convert_js(bound_js);
        js["geometricError"] = m_geometricError;
        switch (m_refineStyle) {
            case ADD: js["refine"] = "ADD"; break;
            case REPLACE: js["refine"] = "REPLACE"; break;
        };
        if (!m_content.uri.empty()) { js["content"]["url"] = m_content.uri; }
        // std::vector<float> trans_data;
        // ygl::mat_to_array(trans_data, m_transform);
        js["transform"] = m_transform;
        for (auto sonNode : m_childNode) {
            json son_js;
            js["children"].push_back(sonNode->convert_js(son_js));
        }
        return js;
    };
    void save_b3dm(const std::string& basePath) {
        if (!m_content.uri.empty() && m_b3dm) {
            std::string filename = basePath + m_content.uri;
            auto f = fopen(filename.c_str(), "wb");
            if (!f) throw std::runtime_error("cannot write file " + filename);
            m_b3dm->save(f);
            fclose(f);
        }
        for (auto sonNode : m_childNode) { sonNode->save_b3dm(basePath); }
    }
};
struct Asset {
    std::string m_version = "0.0";
    std::string m_tilesetVersion = "beyon-0.0";
    std::string m_glftUpAxis = "Y";
};
struct Tileset {
    Asset m_asset;
    float m_geometricError = 200;
    TileNode* m_root = nullptr;
    std::string m_basePath = "";
    std::string m_gltfUpAxis = "Y";
    RefineStyle m_refineStyle = ADD;
    json& convert_js(json& js) {
        js["asset"] = {{"version", "0.0"}, {"tilesetVersion", "beyon_3dtile"},
            {"gltfUpAxis", m_gltfUpAxis}};
        js["geometricError"] = m_geometricError;

        switch (m_refineStyle) {
            case ADD: js["refine"] = "ADD"; break;
            case REPLACE: js["refine"] = "REPLACE"; break;
        };
        js["refine"] = m_geometricError;
        json roo_js;
        js["root"] = m_root->convert_js(roo_js);
        return js;
    }
};
class Beyon3dtile {
   public:
    Beyon3dtile(TileType type);
    ~Beyon3dtile();

   public:
    TileType m_tileType = B3DM;
    Tileset* m_tileSet = nullptr;
    glTF* m_gltf = nullptr;

   public:
    void save() {
        if (save_tileset()) save_model();
    };
    bool generate_tileset(glTF* gl, std::string outpath, double radian_x,
        double radian_y, double height, double tile_w = 200,
        double tile_h = 200, double height_min = 0, double height_max = 100,
        double geometricError = 200);
    void set_pos(double lon, double lat, double height);

   private:
    void save_model();
    bool save_tileset();
};

};  // namespace ygl

#endif