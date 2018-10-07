// Copyright 2008-2016 Conrad Sanderson (http://conradsanderson.id.au)
// Copyright 2008-2016 National ICT Australia (NICTA)
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ------------------------------------------------------------------------


//! \addtogroup spglue_schur
//! @{



template<typename T1, typename T2>
arma_hot
inline
void
spglue_schur::apply(SpMat<typename T1::elem_type>& out, const SpGlue<T1,T2,spglue_schur>& X)
  {
  arma_extra_debug_sigprint();
  
  typedef typename T1::elem_type eT;
  
  const SpProxy<T1> pa(X.A);
  const SpProxy<T2> pb(X.B);
  
  const bool is_alias = pa.is_alias(out) || pb.is_alias(out);
  
  if(is_alias == false)
    {
    spglue_schur::apply_noalias(out, pa, pb);
    }
  else
    {
    SpMat<eT> tmp;
    spglue_schur::apply_noalias(tmp, pa, pb);
    
    out.steal_mem(tmp);
    }
  }



template<typename eT, typename T1, typename T2>
arma_hot
inline
void
spglue_schur::apply_noalias(SpMat<eT>& out, const SpProxy<T1>& pa, const SpProxy<T2>& pb)
  {
  arma_extra_debug_sigprint();
  
  arma_debug_assert_same_size(pa.get_n_rows(), pa.get_n_cols(), pb.get_n_rows(), pb.get_n_cols(), "element-wise multiplication");
  
  if( (pa.get_n_nonzero() != 0) && (pb.get_n_nonzero() != 0) )
    {
    const uword max_n_nonzero = spglue_elem_helper::max_n_nonzero_schur(pa, pb);
    
    // Resize memory to upper bound
    out.reserve(pa.get_n_rows(), pa.get_n_cols(), max_n_nonzero);
    
    // Now iterate across both matrices.
    typename SpProxy<T1>::const_iterator_type x_it  = pa.begin();
    typename SpProxy<T1>::const_iterator_type x_end = pa.end();
    
    typename SpProxy<T2>::const_iterator_type y_it  = pb.begin();
    typename SpProxy<T2>::const_iterator_type y_end = pb.end();
    
    const uword n_rows = pa.get_n_rows();
    
    uword count = 0;
    
    while( (x_it != x_end) || (y_it != y_end) )
      {
      const uword x_it_row = x_it.row();
      const uword x_it_col = x_it.col();
      
      const uword x_index  = x_it_row + x_it_col * n_rows;
      
      const uword y_it_row = y_it.row();
      const uword y_it_col = y_it.col();
      
      const uword y_index  = y_it_row + y_it_col * n_rows;
      
      if(x_index == y_index)
        {
        const eT out_val = (*x_it) * (*y_it);
        
        if(out_val != eT(0))
          {
          access::rw(out.values[count]) = out_val;
          
          access::rw(out.row_indices[count]) = x_it_row;
          access::rw(out.col_ptrs[x_it_col + 1])++;
          ++count;
          }
        
        ++x_it;
        ++y_it;
        }
      else
        {
        if(x_index < y_index)
          {
          ++x_it;
          }
        else
          {
          ++y_it;
          }
        }
      }
    
    const uword out_n_cols = out.n_cols;
    
    uword* col_ptrs = access::rwp(out.col_ptrs);
    
    // Fix column pointers to be cumulative.
    for(uword c = 1; c <= out_n_cols; ++c)
      {
      col_ptrs[c] += col_ptrs[c - 1];
      }
    
    if(count < max_n_nonzero)
      {
      if(count <= (max_n_nonzero/2))
        {
        out.mem_resize(count);
        }
      else
        {
        // quick resize without reallocating memory and copying data
        access::rw(         out.n_nonzero) = count;
        access::rw(     out.values[count]) = eT(0);
        access::rw(out.row_indices[count]) = uword(0);
        }
      }
    }
  else
    {
    out.zeros(pa.get_n_rows(), pa.get_n_cols());
    }
  }



//! @}
