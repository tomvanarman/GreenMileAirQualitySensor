import mariadb from 'mariadb';

let pool: mariadb.Pool;

/**
 * Get a database connection from the connection pool.
 * Creates a new connection pool if none exists.
 */
export default function db() {
    if (pool === undefined) {
        pool = Object.freeze(
            mariadb.createPool({
                host: process.env.MARIADB_HOST,
                user: process.env.MARIADB_USER,
                password: process.env.MARIADB_PASSWORD,
                database: process.env.MARIADB_DATABASE,
                connectionLimit: 5,
            })
        );
    }

    return pool;
}
