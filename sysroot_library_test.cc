//
// sysroot_libary_test.cc
//
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <numeric> // For std::iota
#include <stdexcept> // For std::runtime_error
#include <cstring>   // For strlen, memcmp
#include <cstdio>    // For FILE*, fopen, fclose, printf
#include <cstdlib>   // For malloc, free
#include <cmath>     // For sqrt in image generation
#include <iomanip>   // For std::hex, std::setw, std::setfill
#include <sstream>   // For std::ostringstream
#include <csetjmp>   // For jmp_buf with libjpeg

// Library headers
#include <bzlib.h>
#include <zstd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h> // For xmlBuffer functions
#include <jansson.h>
#include <openssl/evp.h>
#include <openssl/sha.h> // For SHA256_DIGEST_LENGTH
#include <pcre.h>

#define PCRE2_CODE_UNIT_WIDTH 8 // For 8-bit library pcre2-8
#include <pcre2.h>

#include <re2/re2.h>
#include <png.h>
#include <jpeglib.h>

// --- Standard C++ Features Test ---
void test_std_features() {
    std::cout << "\n--- Testing Standard C++ Features ---" << std::endl;
    bool success = true;
    try {
        std::vector<int> v(5);
        std::iota(v.begin(), v.end(), 1);
        std::sort(v.rbegin(), v.rend());
        std::cout << "Sorted vector: ";
        for (int i : v) std::cout << i << " ";
        std::cout << std::endl;
        if (v.empty() || v[0] != 5 || v[4] != 1) {
            std::cerr << "ERROR: Vector sort failed." << std::endl;
            success = false;
        }

        std::string s = "hello";
        s += " world";
        std::cout << "String: " << s << std::endl;
        if (s != "hello world") {
            std::cerr << "ERROR: String manipulation failed." << std::endl;
            success = false;
        }

        std::map<std::string, int> m;
        m["one"] = 1;
        m["two"] = 2;
        std::cout << "Map content for 'one': " << m["one"] << std::endl;
        if (m.find("one") == m.end() || m["one"] != 1 || m.size() != 2) {
            std::cerr << "ERROR: Map operations failed." << std::endl;
            success = false;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR in test_std_features: " << e.what() << std::endl;
        success = false;
    }
    std::cout << "Std features test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_std_features failed");
}

// --- Bzip2 Test ---
void test_bzip2() {
    std::cout << "\n--- Testing Bzip2 ---" << std::endl;
    bool success = true;
    const char* original_data = "This is a test string for bzip2 compression. Repeat: test string for bzip2.";
    unsigned int original_len = strlen(original_data) + 1; // Include null terminator for simple comparison
    std::vector<char> compressed_data(original_len + 100); // Sufficient buffer
    unsigned int compressed_len = compressed_data.size();
    std::vector<char> decompressed_data(original_len);
    unsigned int decompressed_len = decompressed_data.size();

    int ret = BZ2_bzBuffToBuffCompress(compressed_data.data(), &compressed_len,
                                     (char*)original_data, original_len,
                                     9, 0, 30);
    if (ret != BZ_OK) {
        std::cerr << "ERROR: Bzip2 compression failed. Code: " << ret << std::endl;
        success = false;
    } else {
        std::cout << "Bzip2: Original size: " << original_len << ", Compressed size: " << compressed_len << std::endl;
        ret = BZ2_bzBuffToBuffDecompress(decompressed_data.data(), &decompressed_len,
                                         compressed_data.data(), compressed_len,
                                         0, 0);
        if (ret != BZ_OK) {
            std::cerr << "ERROR: Bzip2 decompression failed. Code: " << ret << std::endl;
            success = false;
        } else if (decompressed_len != original_len || memcmp(original_data, decompressed_data.data(), original_len) != 0) {
            std::cerr << "ERROR: Bzip2 decompression mismatch." << std::endl;
            success = false;
        } else {
            std::cout << "Bzip2: Decompression successful." << std::endl;
        }
    }
    std::cout << "Bzip2 test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_bzip2 failed");
}

// --- Zstd Test ---
void test_zstd() {
    std::cout << "\n--- Testing Zstd ---" << std::endl;
    bool success = true;
    const char* original_data = "This is a test string for zstd compression. Repeat: test string for zstd.";
    size_t original_len = strlen(original_data) + 1; // Include null terminator
    size_t compressed_bound = ZSTD_compressBound(original_len);
    std::vector<char> compressed_data(compressed_bound);
    std::vector<char> decompressed_data(original_len);

    size_t compressed_size = ZSTD_compress(compressed_data.data(), compressed_bound,
                                           original_data, original_len, 1);
    if (ZSTD_isError(compressed_size)) {
        std::cerr << "ERROR: Zstd compression failed: " << ZSTD_getErrorName(compressed_size) << std::endl;
        success = false;
    } else {
        std::cout << "Zstd: Original size: " << original_len << ", Compressed size: " << compressed_size << std::endl;
        size_t decompressed_size = ZSTD_decompress(decompressed_data.data(), original_len,
                                                 compressed_data.data(), compressed_size);
        if (ZSTD_isError(decompressed_size)) {
            std::cerr << "ERROR: Zstd decompression failed: " << ZSTD_getErrorName(decompressed_size) << std::endl;
            success = false;
        } else if (decompressed_size != original_len || memcmp(original_data, decompressed_data.data(), original_len) != 0) {
            std::cerr << "ERROR: Zstd decompression mismatch." << std::endl;
            success = false;
        } else {
            std::cout << "Zstd: Decompression successful." << std::endl;
        }
    }
    std::cout << "Zstd test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_zstd failed");
}

// --- libxml2 Test ---
void test_libxml2() {
    std::cout << "\n--- Testing libxml2 ---" << std::endl;
    bool success = true;
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    LIBXML_TEST_VERSION;

    doc = xmlNewDoc(BAD_CAST "1.0");
    if (!doc) {
        std::cerr << "ERROR: libxml2: Failed to create new document." << std::endl;
        success = false;
    } else {
        root_node = xmlNewNode(NULL, BAD_CAST "root");
        xmlDocSetRootElement(doc, root_node);
        xmlNewChild(root_node, NULL, BAD_CAST "element", BAD_CAST "text content");
        xmlNewProp(xmlFirstElementChild(root_node), BAD_CAST "attribute", BAD_CAST "value");

        xmlBufferPtr buffer = xmlBufferCreate();
        if (!buffer) {
             std::cerr << "ERROR: libxml2: Failed to create XML buffer." << std::endl;
             success = false;
        } else {
            int bytesWritten = xmlNodeDump(buffer, doc, root_node, 0, 1); // Level 0, pretty print 1
            if (bytesWritten > 0) {
                std::cout << "Generated XML:\n" << (const char*)xmlBufferContent(buffer) << std::endl;
                std::string xml_str = (const char*)xmlBufferContent(buffer);
                if (xml_str.find("<root>") == std::string::npos || xml_str.find("attribute=\"value\"") == std::string::npos) {
                    std::cerr << "ERROR: libxml2: XML content validation failed." << std::endl;
                    success = false;
                }
            } else {
                std::cerr << "ERROR: libxml2: xmlNodeDump wrote 0 bytes." << std::endl;
                success = false;
            }
            xmlBufferFree(buffer);
        }
        xmlFreeDoc(doc);
    }
    xmlCleanupParser();
    std::cout << "libxml2 test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_libxml2 failed");
}

// --- Jansson Test ---
void test_jansson() {
    std::cout << "\n--- Testing Jansson ---" << std::endl;
    bool success = true;
    json_t *root = json_object();
    json_t *nested = json_object();
    char *json_str = NULL;

    if (!root || !nested) {
        std::cerr << "ERROR: Jansson: Failed to create JSON objects." << std::endl;
        if (root) json_decref(root);
        if (nested) json_decref(nested); // only if not set to root
        success = false;
    } else {
        json_object_set_new(root, "name", json_string("sysroot_test"));
        json_object_set_new(root, "version", json_integer(1));
        json_object_set_new(nested, "detail", json_string("some_detail_value"));
        json_object_set_new(root, "data", nested); // nested is consumed

        json_str = json_dumps(root, JSON_INDENT(2) | JSON_SORT_KEYS);
        if (!json_str) {
            std::cerr << "ERROR: Jansson: Failed to dump JSON to string." << std::endl;
            success = false;
        } else {
            std::cout << "Generated JSON:\n" << json_str << std::endl;
            std::string s(json_str);
            if (s.find("\"name\": \"sysroot_test\"") == std::string::npos || s.find("\"detail\": \"some_detail_value\"") == std::string::npos) {
                std::cerr << "ERROR: Jansson: JSON content validation failed." << std::endl;
                success = false;
            }
            free(json_str);
        }
        json_decref(root); // root decrefs nested
    }
    std::cout << "Jansson test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_jansson failed");
}

// --- OpenSSL SHA256 Test ---
void test_openssl_sha256() {
    std::cout << "\n--- Testing OpenSSL (SHA256) ---" << std::endl;
    bool success = true;
    const char* message = "Test message for OpenSSL SHA256 hashing.";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* mdctx = NULL;
    const EVP_MD* md = EVP_get_digestbyname("SHA256");

    if (md == NULL) {
        std::cerr << "ERROR: OpenSSL: EVP_get_digestbyname failed for SHA256." << std::endl;
        success = false;
    } else {
        mdctx = EVP_MD_CTX_new();
        if (mdctx == NULL) {
            std::cerr << "ERROR: OpenSSL: EVP_MD_CTX_new failed." << std::endl;
            success = false;
        } else {
            if (1 != EVP_DigestInit_ex(mdctx, md, NULL) ||
                1 != EVP_DigestUpdate(mdctx, message, strlen(message))) {
                std::cerr << "ERROR: OpenSSL: Digest init or update failed." << std::endl;
                success = false;
            } else {
                unsigned int hash_len;
                if (1 != EVP_DigestFinal_ex(mdctx, hash, &hash_len)) {
                    std::cerr << "ERROR: OpenSSL: EVP_DigestFinal_ex failed." << std::endl;
                    success = false;
                } else if (hash_len != SHA256_DIGEST_LENGTH) {
                    std::cerr << "ERROR: OpenSSL: Hash length mismatch." << std::endl;
                    success = false;
                } else {
                    std::ostringstream oss;
                    for (unsigned int i = 0; i < hash_len; i++) {
                        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
                    }
                    std::cout << "SHA256 Hash: " << oss.str() << std::endl;
                    std::string expected_hash = "2b8729ba69a5a30088759e4860f79003009779609211489858038513741839f1";
                    if (oss.str() != expected_hash) {
                        std::cerr << "ERROR: OpenSSL: SHA256 hash mismatch. Expected: " << expected_hash << std::endl;
                        success = false;
                    } else {
                        std::cout << "OpenSSL SHA256 hash matches expected value." << std::endl;
                    }
                }
            }
            EVP_MD_CTX_free(mdctx);
        }
    }
    std::cout << "OpenSSL (SHA256) test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_openssl_sha256 failed");
}

// --- PCRE Test ---
void test_pcre() {
    std::cout << "\n--- Testing PCRE ---" << std::endl;
    bool success = true;
    const char *pattern = "^(\\w+):(\\d+)$";
    const char *subject = "name:1234";
    pcre *re;
    const char *error;
    int erroffset;
    int ovector[30];

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        std::cerr << "ERROR: PCRE compilation failed at offset " << erroffset << ": " << error << std::endl;
        success = false;
    } else {
        int rc = pcre_exec(re, NULL, subject, strlen(subject), 0, 0, ovector, sizeof(ovector)/sizeof(int));
        if (rc < 0) {
            std::cerr << "ERROR: PCRE matching error: " << (rc == PCRE_ERROR_NOMATCH ? "No match" : std::to_string(rc)) << std::endl;
            success = false;
        } else if (rc < 3) { // Expected full match + 2 captures
            std::cerr << "ERROR: PCRE: Incorrect number of captures: " << rc-1 << std::endl;
            success = false;
        } else {
            std::cout << "PCRE: Match successful." << std::endl;
            std::string cap1(subject + ovector[2], ovector[3] - ovector[2]);
            std::string cap2(subject + ovector[4], ovector[5] - ovector[4]);
            std::cout << "  Capture 1: " << cap1 << std::endl;
            std::cout << "  Capture 2: " << cap2 << std::endl;
            if (cap1 != "name" || cap2 != "1234") {
                std::cerr << "ERROR: PCRE: Capture content mismatch." << std::endl;
                success = false;
            }
        }
        pcre_free(re);
    }
    std::cout << "PCRE test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_pcre failed");
}

// --- PCRE2 Test ---
void test_pcre2() {
    std::cout << "\n--- Testing PCRE2 (8-bit) ---" << std::endl;
    bool success = true;
    PCRE2_SPTR pattern = (PCRE2_SPTR)"^item:(\\d{4})$";
    PCRE2_SPTR subject = (PCRE2_SPTR)"item:5678";
    pcre2_code *re;
    int errorcode;
    PCRE2_SIZE erroffset;

    re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);
    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        std::cerr << "ERROR: PCRE2 compilation failed at offset " << erroffset << ": " << buffer << std::endl;
        success = false;
    } else {
        pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
        if (!match_data) {
             std::cerr << "ERROR: PCRE2: pcre2_match_data_create_from_pattern failed." << std::endl;
             success = false;
        } else {
            int rc = pcre2_match(re, subject, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);
            if (rc < 0) {
                std::cerr << "ERROR: PCRE2 matching error: " << (rc == PCRE2_ERROR_NOMATCH ? "No match" : std::to_string(rc)) << std::endl;
                success = false;
            } else if (rc < 2) { // Expected full match + 1 capture
                 std::cerr << "ERROR: PCRE2: Incorrect number of captures: " << rc-1 << std::endl;
                 success = false;
            }
            else {
                std::cout << "PCRE2: Match successful." << std::endl;
                PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
                std::string cap1((const char*)(subject + ovector[2]), ovector[3] - ovector[2]);
                std::cout << "  Capture 1: " << cap1 << std::endl;
                if (cap1 != "5678") {
                    std::cerr << "ERROR: PCRE2: Capture content mismatch." << std::endl;
                    success = false;
                }
            }
            pcre2_match_data_free(match_data);
        }
        pcre2_code_free(re);
    }
    std::cout << "PCRE2 test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_pcre2 failed");
}

// --- RE2 Test ---
void test_re2() {
    std::cout << "\n--- Testing RE2 ---" << std::endl;
    bool success = true;
    std::string text = "key:value90";
    std::string word_capture;
    int num_capture;

    if (RE2::FullMatch(text, "(\\w+):value(\\d+)", &word_capture, &num_capture)) {
        std::cout << "RE2: Full match successful." << std::endl;
        std::cout << "  Capture 1: " << word_capture << std::endl;
        std::cout << "  Capture 2: " << num_capture << std::endl;
        if (word_capture != "key" || num_capture != 90) {
            std::cerr << "ERROR: RE2: Capture content mismatch." << std::endl;
            success = false;
        }
    } else {
        std::cerr << "ERROR: RE2: No full match found." << std::endl;
        success = false;
    }
    std::cout << "RE2 test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_re2 failed");
}

// --- libpng Test ---
void write_png_file(const char* filename, int width, int height, png_bytep* row_pointers) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) throw std::runtime_error("libpng: Could not open file for writing.");

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) { fclose(fp); throw std::runtime_error("libpng: png_create_write_struct failed."); }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fclose(fp); png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        throw std::runtime_error("libpng: png_create_info_struct failed.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp); png_destroy_write_struct(&png_ptr, &info_ptr);
        throw std::runtime_error("libpng: Error during png setup or write.");
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    std::cout << "libpng: Successfully wrote " << filename << std::endl;
}

void test_libpng() {
    std::cout << "\n--- Testing libpng ---" << std::endl;
    bool success = true;
    const int width = 100, height = 100, cx = 50, cy = 50, radius = 40;
    const char* filename = "circle.png";
    png_bytep* row_pointers = NULL;

    try {
        row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
        if (!row_pointers) throw std::runtime_error("libpng: Malloc failed for row_pointers.");
        for (int y = 0; y < height; y++) {
            row_pointers[y] = (png_byte*)malloc(width * 3); // RGB
            if (!row_pointers[y]) throw std::runtime_error("libpng: Malloc failed for row.");
            for (int x = 0; x < width; x++) {
                png_byte* ptr = &(row_pointers[y][x * 3]);
                int dx = x - cx, dy = y - cy;
                if (dx * dx + dy * dy <= radius * radius) { // Red circle
                    ptr[0] = 255; ptr[1] = 0; ptr[2] = 0;
                } else { // White background
                    ptr[0] = 255; ptr[1] = 255; ptr[2] = 255;
                }
            }
        }
        write_png_file(filename, width, height, row_pointers);
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        success = false;
    }

    if (row_pointers) {
        for (int y = 0; y < height; y++) if (row_pointers[y]) free(row_pointers[y]);
        free(row_pointers);
    }
    std::cout << "libpng test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_libpng failed");
}

// --- libjpeg Test ---
struct my_jpeg_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

METHODDEF(void) my_jpeg_error_exit (j_common_ptr cinfo) {
  my_jpeg_error_mgr* myerr = (my_jpeg_error_mgr*) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

void test_libjpeg() {
    std::cout << "\n--- Testing libjpeg ---" << std::endl;
    bool success = true;
    const int width = 100, height = 100, quality = 90, cx = 50, cy = 50, radius = 40;
    const char* filename = "circle.jpg";
    JSAMPLE* image_buffer = NULL;
    JSAMPROW* row_pointer = NULL; // Array of pointers to rows

    struct jpeg_compress_struct cinfo;
    struct my_jpeg_error_mgr jerr;
    FILE* outfile = NULL;

    try {
        image_buffer = (JSAMPLE*)malloc(width * height * 3); // RGB
        row_pointer = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
        if (!image_buffer || !row_pointer) throw std::runtime_error("libjpeg: Malloc failed for image buffers.");

        for (int y = 0; y < height; y++) {
            row_pointer[y] = &image_buffer[y * width * 3];
            for (int x = 0; x < width; x++) {
                JSAMPLE* ptr = &row_pointer[y][x * 3];
                int dx = x - cx, dy = y - cy;
                if (dx * dx + dy * dy <= radius * radius) { // Red circle
                    ptr[0] = 255; ptr[1] = 0; ptr[2] = 0;
                } else { // White background
                    ptr[0] = 255; ptr[1] = 255; ptr[2] = 255;
                }
            }
        }

        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = my_jpeg_error_exit;
        if (setjmp(jerr.setjmp_buffer)) {
            throw std::runtime_error("libjpeg: Error during compression.");
        }

        jpeg_create_compress(&cinfo);
        outfile = fopen(filename, "wb");
        if (!outfile) throw std::runtime_error("libjpeg: Cannot open output file.");
        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);
        jpeg_start_compress(&cinfo, TRUE);

        while (cinfo.next_scanline < cinfo.image_height) {
            jpeg_write_scanlines(&cinfo, &row_pointer[cinfo.next_scanline], 1);
        }

        jpeg_finish_compress(&cinfo);
        std::cout << "libjpeg: Successfully wrote " << filename << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        success = false;
    }

    if (outfile) fclose(outfile);
    jpeg_destroy_compress(&cinfo); // Safe to call even if not fully initialized due to setjmp
    if (image_buffer) free(image_buffer);
    if (row_pointer) free(row_pointer);

    std::cout << "libjpeg test " << (success ? "PASSED" : "FAILED") << std::endl;
    if (!success) throw std::runtime_error("test_libjpeg failed");
}


// --- Main Function ---
int main() {
    std::cout << "Sysroot Test Application Starting..." << std::endl;
    int failed_tests = 0;
    int total_tests = 0;

    auto run_test = [&](const std::string& test_name, void (*test_func)()) {
        total_tests++;
        std::cout << "\n=========================================" << std::endl;
        std::cout << "Starting Test: " << test_name << std::endl;
        std::cout << "=========================================" << std::endl;
        try {
            test_func();
        } catch (const std::runtime_error& e) {
            // Error already printed by the test function or its sub-functions
            // std::cerr << "--- TEST " << test_name << " FAILED WITH EXCEPTION: " << e.what() << " ---" << std::endl;
            failed_tests++;
        } catch (...) {
            std::cerr << "--- TEST " << test_name << " FAILED WITH UNKNOWN EXCEPTION ---" << std::endl;
            failed_tests++;
        }
    };

    run_test("Standard C++ Features", test_std_features);
    run_test("Bzip2", test_bzip2);
    run_test("Zstd", test_zstd);
    run_test("libxml2", test_libxml2);
    run_test("Jansson", test_jansson);
    run_test("OpenSSL SHA256", test_openssl_sha256);
    run_test("PCRE", test_pcre);
    run_test("PCRE2", test_pcre2);
    run_test("RE2", test_re2);
    run_test("libpng", test_libpng);
    run_test("libjpeg", test_libjpeg);

    std::cout << "\n\n--- Test Summary ---" << std::endl;
    std::cout << "Total tests run: " << total_tests << std::endl;
    std::cout << "Tests passed: " << (total_tests - failed_tests) << std::endl;
    std::cout << "Tests failed: " << failed_tests << std::endl;

    if (failed_tests > 0) {
        std::cout << "\nSYSROOT TEST APPLICATION FAILED" << std::endl;
        return 1;
    }

    std::cout << "\nSYSROOT TEST APPLICATION PASSED SUCCESSFULLY" << std::endl;
    return 0;
}

