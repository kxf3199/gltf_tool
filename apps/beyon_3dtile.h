
#include "../yocto/ext/json.hpp"
#include "../yocto/yocto_gl.h"
using json = nlohmann::json;
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
	std::string& align() {
        std::string js_str=js.dump(2);
        while (js_str.length() % 4) js_str += " ";
        return js_str;
	}
    bool save(FILE* f) {
        if (js.empty()) return false;
        std::string js_str = align();
        fwrite(js_str.c_str(), 1, (int)js_str.size(), f);
        fwrite(bin.data(), 1, bin.size(), f);
        return true;
    };
};
struct FeatureTable : TileTable {
    json js = {{"batchLen", 0}};  // batchLen = 0;   //required
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
    B3dModel(const glTF* gl,FeatureTable& ft,BatchTable& bt);
    ~B3dModel();
    B3dmHeader header;
    FeatureTable featureTable;
    BatchTable batchTable;
    const glTF* gltf = nullptr;
    void save(FILE* f) {
        if (header.save(f)) {
            featureTable.save(f);
            batchTable.save(f);
            ygl::save_binary_gltf(f, gltf);
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
    json& convert_js() {
        json js;
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
    mat4f m_transform = {
        {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    float m_geometricError;
    RefineStyle m_refineStyle = ADD;
    TileContent m_content;
    B3dModel* m_b3dm = nullptr;
    std::vector<TileNode*> m_childNode;
    json& convert_js() {
        json js;
        js["boundingVolume"] = m_boundingVol.convert_js();
        js["geometricError"] = 0.0;
        switch (m_refineStyle) {
            case ADD: js["refine"] = "ADD"; break;
            case REPLACE: js["refine"] = "REPLACE"; break;
        };
        if (!m_content.uri.empty()) { 
            js["content"]["uri"] = m_content.uri;
		}
        std::vector<float> trans_data;
        ygl::mat_to_array(trans_data, m_transform);
        js["transform"] = trans_data;
        for (auto sonNode : m_childNode) {
            js["children"].push_back(sonNode->convert_js());
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
    std::string m_tilesetVersion="beyon-0.0";
    std::string m_glftUpAxis = "Y";
};
struct Tileset {
    Asset m_asset;
    float m_geometricError = 200;
    TileNode* m_root=nullptr;
    mat4f m_transform = {
        {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    std::string m_basePath = "";
    char m_gltfUpAxis = 'Y';
    RefineStyle m_refineStyle = ADD;
    json& convert_js() {
        json js;
        js["asset"] = {{"version", "0.0"}, {"tilesetVersion", "beyon_3dtile"},
            {"gltfUpAxis", m_gltfUpAxis}};
        js["geometricError"] = m_geometricError;

        switch (m_refineStyle) {
            case ADD: js["refine"] = "ADD"; break;
            case REPLACE: js["refine"] = "REPLACE"; break;
        };
        js["refine"] = m_geometricError;
        std::vector<float> trans_data;
        ygl::mat_to_array(trans_data, m_transform);
        js["transform"] = trans_data;
        return js;
	}
};
class Beyon3dtile {
    Beyon3dtile(TileType type, Tileset& ts);
    ~Beyon3dtile();
    TileType m_tileType = B3DM;
    Tileset* m_tileSet = nullptr;

   public:
    void save() {
        if (save_tileset()) save_model();
    };

   private:
    void save_model();
    bool save_tileset();
};

};  // namespace ygl