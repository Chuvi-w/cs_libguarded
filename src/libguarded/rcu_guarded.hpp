/*
 * Copyright 2016-2017 Ansel Sermersheim
 *
 * All rights reserved
 *
 * This file is part of libguarded. Libguarded is free software
 * released under the BSD 2-clause license. For more information see
 * the LICENCE file provided with this project.
 */

#ifndef INCLUDED_LIBGUARDED_RCU_GUARDED_HPP
#define INCLUDED_LIBGUARDED_RCU_GUARDED_HPP

#include <memory>

namespace libguarded
{

template <typename T>
class rcu_guarded
{
  private:
  public:
    class write_handle;
    class read_handle;

    template <typename... Us>
    rcu_guarded(Us &&... data);

    // write access
    write_handle lock_write();

    // read access
    read_handle lock_read() const;

    class write_handle
    {
      public:
        using pointer      = T *;
        using element_type = T;

        write_handle(T * ptr) : m_ptr(ptr), m_accessed(false)
        {
        }

        ~write_handle()
        {
            if (m_accessed) {
                m_guard.rcu_write_unlock(*m_ptr);
            }
        };

        T & operator*() const
        {
            access();
            return *m_ptr;
        }

        T * operator->() const
        {
            access();
            return m_ptr;
        }

      private:
        void access() const
        {
            if (!m_accessed) {
                m_guard.rcu_write_lock(*m_ptr);
                m_accessed = true;
            }
        }

        T * m_ptr;
        mutable typename T::rcu_write_guard m_guard;
        mutable bool m_accessed;
    };

    class read_handle
    {
      public:
        using pointer      = const T *;
        using element_type = const T;

        read_handle(const T * ptr) : m_ptr(ptr), m_accessed(false)
        {
        }

        ~read_handle()
        {
            if (m_accessed) {
                m_guard.rcu_read_unlock(*m_ptr);
            }
        };

        const T & operator*() const
        {
            access();
            return *m_ptr;
        }

        const T * operator->() const
        {
            access();
            return m_ptr;
        }

      private:
        void access() const
        {
            if (!m_accessed) {
                m_guard.rcu_read_lock(*m_ptr);
                m_accessed = true;
            }
        }

        const T * m_ptr;
        mutable typename T::rcu_read_guard m_guard;
        mutable bool m_accessed;
    };

  private:
    T m_obj;
};

template <typename T>
template <typename... Us>
rcu_guarded<T>::rcu_guarded(Us &&... data)
    : m_obj(std::forward<Us>(data)...)
{
}

template <typename T>
auto rcu_guarded<T>::lock_write() -> write_handle
{
    return write_handle(&m_obj);
}

template <typename T>
auto rcu_guarded<T>::lock_read() const -> read_handle
{
    return read_handle(&m_obj);
}
}

#endif