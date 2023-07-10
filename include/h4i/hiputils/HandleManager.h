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

    // A map for tracking which HIP stream is associated with each
    // library-specific handle, so that we can respond to 
    // GetStream requests.
    std::unordered_map<HandleType, hipStream_t> handleToHipStreamMap;

public:
    StatusType Create(HandleType* handle)
    {
        if(handle != nullptr)
        {
            *handle = nullptr;

            // Determine the backend we're using.
            auto backend = H4I::MKLShim::Context::ToBackend(hipGetBackendName());

            // Obtain the native backend handles for the default HIP stream.
            StreamHandles streamHandles;

            // Associate the backend handles with the given handle variable.
            *handle = H4I::MKLShim::Context::Create(streamHandles.handles, backend);

            // Add an entry to our map indicating the library-specific handle
            // we just created is associated with the default stream.
            handleToHipStreamMap[*handle] = nullptr;
        }

        // TODO this doesn't handle the case where there was a failure to create.
        // It returns nullHandleStatus in that case, which isn't correct.
        return ((handle != nullptr) and (*handle != nullptr)) ? successStatus : nullHandleStatus;
    }

    StatusType SetStream(HandleType handle, hipStream_t stream)
    {
        if(handle != nullptr)
        {
            // Access the real backend handle context associated with the library-specific handle.
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);

            // Obtain the native backend handles associated with the given HIP stream.
            StreamHandles streamHandles(stream);

            // Associate the native backend handles we found with the backend context.
            // NB: we can't just create a new backend context, because we have no way 
            // to change the caller's handle in this function.
            ctxt->SetStream(streamHandles.handles);

            // Associate the given HIP stream handle with the given library-specific handle.
            assert(handleToHipStreamMap.find(handle) != handleToHipStreamMap.end());
            handleToHipStreamMap[handle] = stream;
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }

    StatusType GetStream(HandleType handle, hipStream_t* stream)
    {
        if(handle != nullptr)
        {
            if(stream != nullptr)
            {
                // Determine which HIP stream is currently associated with the
                // given library-specific handle.  The associated HIP stream
                // handle might be nullptr, indicating the library-specific context
                // is using the default HIP stream.
                auto iter = handleToHipStreamMap.find(handle);
                assert(iter != handleToHipStreamMap.end());
                *stream = iter->second;
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
            // Forget about the given library-specific handle.
            auto iter = handleToHipStreamMap.find(handle);
            assert(iter != handleToHipStreamMap.end());
            handleToHipStreamMap.erase(iter);

            // Release the backend handle context.
            H4I::MKLShim::Context* ctxt = static_cast<H4I::MKLShim::Context*>(handle);
            assert(ctxt != nullptr);
            delete ctxt;
        }
        return (handle != nullptr) ? successStatus : nullHandleStatus;
    }
};

} // namespace H4I::HIPUtils

