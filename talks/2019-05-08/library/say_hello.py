#!/usr/bin/env python3

ANSIBLE_METADATA = {
    'metadata_version': '1.0',
    'status': ['preview'],
    'supported_by': 'community',
}

DOCUMENTATION = '''
---
module: say_hello
short_description: Say hello
description:
  - Says hello!
author: "Keith Gaughan (@kgaughan)"
options:
  name:
    description:
      - Your name
    required: true
'''

RETURN = ''

from ansible.module_utils.basic import AnsibleModule


def main():
    module = AnsibleModule(
        argument_spec={
            'name': {'type': 'path', 'required': True},
        },
        supports_check_mode=True,
    )

    name = module.params['name']

    if name == "":
        module.fail_json(msg="You need to tell me who you are!")

    module.exit_json(changed=True, msg=f"Hello, {name}!")


if __name__ == '__main__':
    main()
