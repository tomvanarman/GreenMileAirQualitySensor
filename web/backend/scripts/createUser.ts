import UserModel from '../src/user/user';

import '../src/env';

async function main() {
    const username = process.argv[2];
    const password = process.argv[3];

    const id = await UserModel.createUser(username, password);
    console.log('Created user with ID:', id);
    process.exit(0);
}

main();
