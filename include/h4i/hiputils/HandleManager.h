// Copyright 2021-2023 UT-Battelle
// See LICENSE.txt in the root of the source distribution for license info.
#pragma once

#include <unordered_map>
#include "h4i/mklshim/Context.h"

namespace H4I::HIPUtils
{

template<typename HandleType,
            typename StatusType,
            StatusType successStatus,
            StatusType nullHandleStatus>
class HandleManager
{
private:
    struct StreamHandles
    {
        H4I::MKLShim::Context::NativeHandleArray handles;
        int nHandles;

        StreamHandles(hipStream_t stream = nullptr)
          : nHandles(handles.size())
        {    
            // Note this code uses a chipStar extension to the HIP API.
            // See chipStar documentation for its use.
            // Both Level Zero and OpenCL backends currently require us
            // to pass nHandles = 4, and provide space for at least 4 handles.
            // TODO is there a way to query this info at runtime?
            hipGetBackendNativeHandles(reinterpret_cast<uintptr_t>(stream),
                handles.data(), &nHandles);
        }
    };

    std::unordered_map<hipblasHandle_t, hipStream_t> handleMap;

public:
    StatusType Create(HandleType* handle)
    {
        if(handle != nullptr)
        {
            *handle = nullptr;

            // Determine the backend we're using.
            auto backend = H4I::MKLShim::Context::ToBackend(hipGetBackendName());

            // Obtain the native backend handles.
            StreamHandles streamHandles;

            // Associate the backend handles with the given handle variable.
            *handle = H4I::MKLShim::Context::Create(streamHandles.handles, backend);
        }

        // TODO this doesn't handle the case where there was a failure to create.
        // It returns nullHandleStatus in that case, which isn't correct.
        return ((handle != nullptr) and (*handle != nullptr)) ? successStatus : nullHandleStatus;
    }

    StatusType SetStream(HandleType handle, hipStream_t stream)
    {
        if(handle != nullptr)
        {
            // Access the real backend handle context associated with the given handle.
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);

            // Obtain the native backend handles associated with the given stream.
            StreamHandles streamHandles(stream);

            // Associate the stream's native handle with our context.
            ctxt->SetStream(streamHandles.handles);

            // Associate the given stream handle with the given hipBLAS handle.
            // Note: this may overwrite some other stream.
            handleMap[handle] = stream;
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }

    StatusType GetStream(HandleType handle, hipStream_t* stream)
    {
        if(handle != nullptr)
        {
            // Access the real backend handle context associated with the given handle.
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);

            if(stream != nullptr)
            {
                // See if we have seen a HIP stream associated with this handle.
                // If not, return null to indicate we're using the default stream.
                *stream = nullptr;
                auto iter = handleMap.find(handle);
                if(iter != handleMap.end())
                {
                    *stream = iter->second;
                }
            }
            else
            {
                // TODO what return code is correct when user
                // didn't give us a place to store the result?
            }
        }
        return ((handle != nullptr) and (stream != nullptr)) ? successStatus : nullHandleStatus;
    }

    StatusType Destroy(HandleType handle)
    {
        if(handle != nullptr)
        {
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);
            delete ctxt;
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }
};

} // namespace H4I::HIPUtils

