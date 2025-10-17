#pragma once

#include "quickdb/components/exception.h"

#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <mongocxx/gridfs/bucket.hpp>
#include <mongocxx/pool.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace QDB
{
    /// @brief A class for interacting with a MongoDB GridFS bucket for large file storage.
    class GridFSBucket
    {
    public:
        /// @brief Constructs a GridFSBucket handler. This is typically created via Database::get_gridfs_bucket().
        /// @param client_entry A unique_ptr to the connection pool entry.
        /// @param bucket The underlying mongocxx bucket handle.
        GridFSBucket(std::unique_ptr<mongocxx::pool::entry> client_entry, mongocxx::gridfs::bucket bucket)
            : _client_entry(std::move(client_entry)), _bucket(std::move(bucket))
        {
        }

        /// @brief Uploads a file from the local filesystem to GridFS using streams.
        /// @param filename The name of the file to store in GridFS.
        /// @param source_path The local path to the file to be uploaded.
        /// @return The ObjectId of the newly created file in GridFS.
        bsoncxx::oid upload_from_file(const std::string &filename, const std::string &source_path)
        {
            try
            {
                std::ifstream source_stream(source_path, std::ios::binary);
                if (!source_stream)
                {
                    throw QDB::Exception("GridFS upload failed: could not open source file '" + source_path + "'.");
                }

                auto uploader = _bucket.open_upload_stream(filename);
                char buffer[4096];
                while (source_stream.read(buffer, sizeof(buffer)))
                {
                    uploader.write(reinterpret_cast<const std::uint8_t *>(buffer), source_stream.gcount());
                }
                // Write any remaining bytes from the last read
                if (source_stream.gcount() > 0)
                {
                    uploader.write(reinterpret_cast<const std::uint8_t *>(buffer), source_stream.gcount());
                }

                auto result = uploader.close();
                return result.id().get_oid().value;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("GridFS upload failed for file '" + source_path + "': " + std::string(e.what()));
            }
        }

        /// @brief Downloads a file from GridFS to the local filesystem using streams.
        /// @param file_id The ObjectId of the file to download.
        /// @param destination_path The local path where the file will be saved.
        void download_to_file(bsoncxx::oid file_id, const std::string &destination_path)
        {
            try
            {
                std::ofstream destination_stream(destination_path, std::ios::binary);
                if (!destination_stream)
                {
                    throw QDB::Exception("GridFS download failed: could not open destination file '" + destination_path +
                                         "'.");
                }

                bsoncxx::types::bson_value::value oid_value(bsoncxx::types::b_oid{file_id});
                auto downloader = _bucket.open_download_stream(oid_value.view());
                auto file_length = downloader.file_length();
                std::int64_t bytes_read = 0;
                std::uint8_t buffer[4096];

                while (bytes_read < file_length)
                {
                    auto bytes_to_read = std::min(static_cast<std::int64_t>(sizeof(buffer)), file_length - bytes_read);
                    downloader.read(buffer, bytes_to_read);
                    destination_stream.write(reinterpret_cast<const char *>(buffer), bytes_to_read);
                    bytes_read += bytes_to_read;
                }
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("GridFS download failed for file ID '" + file_id.to_string() +
                                     "': " + std::string(e.what()));
            }
        }

        /// @brief Deletes a file and its associated chunks from GridFS.
        /// @param file_id The ObjectId of the file to delete.
        void delete_file(bsoncxx::oid file_id)
        {
            try
            {
                bsoncxx::types::bson_value::value oid_value(bsoncxx::types::b_oid{file_id});
                _bucket.delete_file(oid_value.view());
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("GridFS delete failed for file ID '" + file_id.to_string() +
                                     "': " + std::string(e.what()));
            }
        }

    private:
        /// @brief This unique_ptr owns the client connection, keeping it alive.
        std::unique_ptr<mongocxx::pool::entry> _client_entry;

        /// @brief The bucket handle itself. It is dependent on the client from _client_entry.
        mongocxx::gridfs::bucket _bucket;
    };
} // namespace QDB
