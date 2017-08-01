#ifndef SN_BUILTIN_GC_PTR_H
#define SN_BUILTIN_GC_PTR_H

#include <memory>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <set>
#include <vector>
#include <mutex>
#include <atomic>

namespace sn_Builtin {
    // ref: vczh/vczh_toys/gc_ptr
    // mark and sweep
    // ref: https://zhuanlan.zhihu.com/p/28220847

    // TODO: type-safety
    // TODO: root-analysis (may choose nullptr)

    // void** handle_ref: gc_ptr<T>
    // void* ref: enable_gc* (object address, namely this)

    namespace gc_ptr {
        class enable_gc;
        
        // record the start of object address, length of continuous object and handle
        struct gc_record {
            void* start = nullptr;
            int length = 0;
            enable_gc* handle = nullptr;
        };
        
        template <typename T>
        class gc_ptr;

        // virtual base for enable gc
        class enable_gc {
            template <typename T>
            friend class gc_ptr;

            template <typename T, typename ...Args>
            friend gc_ptr<T> make_gc(Args&&... args);
        private:
            gc_record m_record;
            void set_record(gc_record) { m_record = record; }
        public:
            enable_gc() {}
            virtual ~enable_gc() {}
        };

        // collect DFS mark and sweep data
        struct gc_handle {
            static const int counter_range = (int32_t)0x80000000;
            int counter = 0;
            gc_record record;
            std::multiset<gc_handle*> refs;
            std::multiset<void**> handle_refs;
            bool mark = false;
        };

        // just for find parent and self from pool
        struct gc_dummy_handle {
            int counter = 0;
            gc_record record;
        };

        struct gc_handle_comparator {
            // !(a<b) && !(b<a) -> a == b
            bool operator()(gc_handle* a, gc_handle* b) const {
                // for dummy gc_ptr a < any b
                if (a->counter == gc_handle::counter_range) {
                    return a->record.start < b->record.start;
                }
                // for any a < dummy gc_ptr b
                if (b->counter == gc_handle::counter_range) {
                    return static_cast<intptr_t>(a->record.start) + a->record.length <= static_cast<intptr_t>(b->record.start);
                }
                // the effect is &dummy gc_ptr a < b(a.member) < &dummy a
                // for non dummy gc_ptr a and b, just compare their start address
                return a->record.start < b->record.start;
            }
        };

        using gc_handle_container = std::set<gc_handle*, gc_handle_comparator>;
        mutex gc_lock;
        gc_handle_container* gc_handles = nullptr;
        std::size_t gc_step_size = 0;
        std::size_t gc_max_size = 0;
        std::size_t gc_last_current_size = 0;
        std::size_t gc_current_size = 0;
        
        // find handle in the global handle pool
        gc_handle* gc_find_unsafe(void* handle) {
            gc_dummy_handle dummy;
            dummy.record.start = handle;
            gc_handle* input = reinterpret_cast<gc_handle*>(&dummy);
            auto it = gc_handles->find(input);
            return it == gc_handles.end() ? nullptr : *it;
        }

        // find parent of current handle_ref(gc_ptr<T>)
        gc_handle* gc_find_parent_unsafe(void** handle_ref) {
            gc_handle_dummy dummy;
            dummy.counter = gc_handle::counter_range;
            dummy.record.start = static_cast<void*>(handle_ref);
            gc_handle* input = reinterpret_cast<gc_handle*>(&dummy);
            auto it = gc_handles->find(input);
            return it == gc_handles->end() ? nullptr : *it;
        }

        // connect parent and childs(gc_ptr<T>)
        // connect gc_ptr<T> and object handle
        void gc_ref_connect_unsafe(void** handle_ref, void* handle, bool alloc) {
            gc_handle* parent = nullptr;
            if (alloc) {
                parent == gc_find_parent_unsafe(handle_ref)
                if (parent) {
                    parent->handle_refs.insert(handle_ref);
                }
            }
            auto target = gc_find_unsafe(handle);
            if (target) {
                if (parent || (parent = gc_find_parent_unsafe(handle_ref))) {
                    parnet->refs.insert(target);
                    if (alloc) {
                        parent->handle_refs.insert(handle_ref);
                    }
                } else {
                    ++target->counter;
                }
            }
        }

        void gc_ref_disconnect_unsafe(void** handle_ref, void* handle, bool dealloc) {
            gc_handle* parent = nullptr;
            if (dealloc) {
                parent == gc_find_parent_unsafe(handle_ref)
                if (parent) {
                    parent->handle_refs.insert(handle_ref);
                }
            }
            auto target = gc_find_unsafe(handle);
            if (target) {
                if (parent || (parent = gc_find_parent_unsafe(handle_ref))) {
                    parnet->refs.erase(target);
                    if (dealloc) {
                        parent->handle_refs.erase(handle_ref);
                    }
                } else {
                    --target->counter;
                }
            }
        }

        // remove connect
        void gc_destroy_disconnect_unsafe(gc_handle* handle) {
            for (auto handle_ref: handle->handle_refs) {
                // for test?
                auto x = reinterpret_cast<gc_ptr<enable_gc>*>(handle_ref);
                *handle_ref = nullptr;
            }
        }

        // destruct object
        void gc_destroy_unsafe(gc_handle* handle) {
            // better new and delete? manually dtor and free may cause unpredicted outcome
            handle->record.handle->~enable_gc();
            gc_current_size -= handle->record.length;
            free(handle->record.start);
            delete handle;
        }

        // destruct member of parents
        void gc_destroy_unsafe(std::vector<gc_handle*>& garbages) {
            for (auto handle: garbages) {
                gc_destroy_disconnect_unsafe(handle);
            }
            for (auto handle: garbages) {
                gc_destroy_unsafe(handle);
            }
            gc_last_current_size = gc_current_size;
        }

        // main mark-and-sweep
        void gc_force_collect_unsafe(std::vector<gc_handle*>& garbages) {
            std::vector<gc_handle*> markings;

            // mark all reachable
            for (auto handle : *gc_handles) {
                if (handle->mark = handle->counter > 0) {
                    markings.push_back(handle);
                }
            }

            for (int i = 0; i < static_cast<int>(markings.size()); i++) {
                auto ref = markings[i];
                for (auto child : ref->references) {
                    if (!child->mark) {
                        child->mark = true;
                        markings.push_back(child);
                    }
                }
            }

            // collect unreachable
            for (auto it = gc_handles->begin(); it != gc_handles->end();) {
                if (!(*it)->mark) {
                    auto it2 = it++;
                    garbages.push_back(*it2);
                    gc_handles->erase(it2);
                }
                else {
                    it++;
                }
            }
        }

        // alloc/dealloc handles
        namespace unsafe {
            // push record into pool
            void gc_alloc(gc_record record) {
                assert(gc_handles);
                auto handle = new gc_handle;
                handle->record = record;
                handle->counter = 1;
                std::vector<gc_handle*> garbages;
                {
                    std::lock_guard<std::mutex> lk{gc_lock};
                    gc_handles->insert(handle);
                    gc_current_size += handle->record.length;
                    if (gc_current_size > gc_max_size) {
                        gc_force_collect_unsafe(garbages);
                    } else if (gc_current_size - gc_last_current_size > gc_step_size) {
                        gc_force_collect_unsafe(garbages);
                    }
                }
                gc_destroy_unsafe(garbages);
            }
            
            void gc_register(void* ref, enable_gc* handle) {
                assert(gc_handles);
                std::lock_guard<std::mutex> lk{gc_lock};
                gc_find_unsafe(ref)->record.handle = handle;
            }

            void gc_ref_alloc(void** handle_ref, void* handle) {
                assert(gc_handles);
                std::lock_guard<std::mutex> lk{gc_lock};
                gc_ref_connect_unsafe(handle_ref, handle, true);
            }
            
            void gc_ref_dealloc(void** handle_ref, void* handle) {
                assert(gc_handles);
                std::lock_guard<std::mutex> lk{gc_lock};
                gc_ref_disconnect_unsafe(handle_ref, handle, true);
            }

            void gc_ref(void** handle_ref, void* old_handle, void* new_handle) {
                assert(gc_handles);
                std::lock_guard<std::mutex> lk{gc_lock};
                gc_ref_disconnect_unsafe(handle_ref, old_handle, false);
                gc_ref_connect_unsafe(handle_ref, new_handle, false);
            }
        }

        void gc_force_collect() {
            assert(gc_handles);
            std::vector<gc_handle*> garbages;
            {
                std::lock_guard<std::mutex> lk{gc_lock};
                gc_force_collect_unsafe(garbages);
            }
            gc_destroy_unsafe(garbages);
        }

        void gc_start(std::size_t step_size, std::size_t max_size) {
            assert(!gc_handles);
            std::lock_guard<std::mutex> lk{gc_lock};
            gc_handles = new gc_handle_container;
            gc_step_size = step_size;
            gc_max_size = max_size;
            gc_last_current_size = 0;
            gc_current_size = 0;
        }

        void gc_stop() {
            assert(gc_handles);
            std::lock_guard<std::mutex> lk{gc_lock};
            gc_force_collect();
            auto garbages = gc_handles;
            gc_handles = nullptr;
            gc_step_size = 0;
            gc_max_size = 0;
            gc_last_current_size = 0;
            gc_current_size = 0;

            for (auto handle: *garbages) {
                gc_destroy_disconnect_unsafe(handle);
            }
            for (auto handle: *garbages) {
                gc_destroy_unsafe(handle);
            }
            delete garbages;
        }
        
        
        // all gc-recorded class T virtual inherit from enable_gc
        // And the class member should in gc_ptr<MemberType>
        template <typename T>
        class gc_ptr {
            template <typename U>
            friend class gc_ptr;

            template <typename U, typename ...Args>
            friend gc_ptr<U> make_gc(Args&&... args);

            template <typename U, typename V>
            friend gc_ptr<U> static_gc_cast(const gc_ptr<V>& ptr);
            
            template <typename U, typename V>
            friend gc_ptr<U> dynamic_gc_cast(const gc_ptr<V>& ptr);
            
        private:
            T* m_ref = nullptr;
            static void* handle_of(T* ref) {
                return m_ref ? static_cast<enable_gc*>(m_ref)->record.start : nullptr;
            }
            gc_ptr(T* ref) : m_ref{ref} {
                unsafe::gc_ref_alloc(static_cast<void**>(this), handle_of(m_ref));
            }
        public:
            gc_ptr() : m_ref{nullptr} {
                unsafe::gc_ref_alloc(static_cast<void**>(this), nullptr);
            }
            gc_ptr(const gc_ptr<T>& ptr) : m_ref{ptr.m_ref} {
                unsafe::gc_ref_alloc(static_cast<void**>(this), handle_of(m_ref));
            }
            gc_ptr(gc_ptr<T>&& ptr) : m_ref{ptr.m_ref} {
                unsafe::gc_ref_alloc(static_cast<void**>(this), handle_of(m_ref));
                ptr.m_ref = nullptr;
                unsafe::gc_ref(static_cast<void**>(&ptr), handle_of(m_ref));
            }
            template <typename U>
            gc_ptr(const gc_ptr<U>& ptr) : m_ref{ptr.m_ref} {
                unsafe::gc_ref_alloc(static_cast<void**>(this), handle_of(m_ref));
            }
            ~gc_ptr() {
                unsafe::gc_ref_dealloc(static_cast<void**>(this), handle_of(m_ref));
            }
            gc_ptr<T>& operator=(const gc_ptr<T>& ptr) {
                void* old_handle = handle_of(m_ref);
                m_ref = ptr.m_ref;
                void* new_handle = handle_of(m_ref);
                unsafe::gc_ref(static_cast<void**>(this), old_handle, new_handle);
                return *this;
            }
            T* operator->() {
                return m_ref;
            }
        };

        // T should be POD (trivial?)
        // However, the normal implementation of virtual table affects the size of class

        /*
            class D -> base class B -> gc_ptr<T> of base class B
            |-------|
            |   D   |
            |--- ---|
            |   B   |
            |  ptr  |
            |-------|

            gc_handles <- &D (memory)
                       .handle_refs <- gc_handle <- record <- nullptr-handle (gc_ptr - ctor - gc_ref_alloc)
            nullptr-handle <- ref
                                                                due to pass local temporary this, cannot find parent of temporary gc_ptr<T>{ref}
                       .handle_refs <- gc_handle <- counter++ (gc_ptr<T>{ref} temporary)
            due to gc_ref
                       .handle_refs <- gc_handle <- counter--
            then copy-elision or operator=
        
        
        */


        template <typename T, typename ...Args>
        gc_ptr<T> make_gc(Args&&... args) {
            void* memory = malloc(sizeof(T));
            gc_record record;
            record.start = memory;
            record.length = sizeof(T);
            unsafe::gc_alloc(record);

            T* ref = new (memory) T{std::forward<Args>(args)...};
            enable_gc* e = static_cast<enable_gc*>(ref);
            record.handle = e;
            e->set_record(record);
            unsafe::gc_register(memory, e);

            auto ptr = gc_ptr<T>{ref};
            unsafe::gc_ref(nullptr, memory, nullptr); // record.counter--
            return ptr;
        }

        template <typename T, typename U>
        gc_ptr<T> static_gc_cast(const gc_ptr<U>& ptr) {
            return gc_ptr<T>{ptr};
        }

        template <typename T, typename U>
        gc_ptr<T> dynamic_gc_cast(const gc_ptr<U>& ptr) {
            return gc_ptr<T>{dynamic_cast<T*>(ptr)};
        }

#define SN_BUILTIN_ENABLE_GC public virtual ::sn_Builtin::gc_ptr::enable_gc
    }
}


#endif