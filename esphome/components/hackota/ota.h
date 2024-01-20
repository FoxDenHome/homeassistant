#include "esphome.h"
#include "esphome/components/ota/ota_component.h"

template<typename Tag>
struct result {
  /* export it ... */
  typedef typename Tag::type type;
  static type ptr;
};

template<typename Tag>
typename result<Tag>::type result<Tag>::ptr;

template<typename Tag, typename Tag::type p>
struct rob : result<Tag> {
  /* fill it ... */
  struct filler {
    filler() { result<Tag>::ptr = p; }
  };
  static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

struct OTAComponent_client_ {
    typedef std::unique_ptr<socket::Socket> esphome::ota::OTAComponent::*type;
};
template class rob<OTAComponent_client_, &esphome::ota::OTAComponent::client_>;
